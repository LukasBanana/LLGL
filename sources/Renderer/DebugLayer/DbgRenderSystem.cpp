/*
 * DbgRenderSystem.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "DbgRenderSystem.h"
#include "DbgCore.h"
#include "DbgReportUtils.h"
#include "../BufferUtils.h"
#include "../TextureUtils.h"
#include "../CheckedCast.h"
#include "../RenderTargetUtils.h"
#include "../../Core/CoreUtils.h"
#include "../../Core/StringUtils.h"
#include <LLGL/ImageFlags.h>
#include <LLGL/Constants.h>
#include <LLGL/Utils/TypeNames.h>
#include <LLGL/Utils/ForRange.h>
#include <unordered_map>


namespace LLGL
{


/*
~~~~~~ INFO ~~~~~~
This is the debug layer render system.
It is a wrapper for the actual render system to validate the parameters, specified by the client programmer.
All the "Create..." and "Write..." functions wrap the function call of the actual render system
into a single braces block to highlight this function call, where the input parameters are just passed on.
All the actual render system objects are stored in the members named "instance", since they are the actual object instances.
*/

DbgRenderSystem::DbgRenderSystem(RenderSystemPtr&& instance, RenderingDebugger* debugger) :
    instance_     { std::forward<RenderSystemPtr&&>(instance)                                         },
    debugger_     { debugger                                                                          },
    commandQueue_ { MakeUnique<DbgCommandQueue>(*(instance_->GetCommandQueue()), profile_, debugger_) }
{
}

void DbgRenderSystem::FlushProfile()
{
    if (debugger_ != nullptr)
        debugger_->RecordProfile(profile_);
    profile_ = {};
}

bool DbgRenderSystem::IsVulkan() const
{
    return (GetRendererID() == RendererID::Vulkan);
}

/* ----- Swap-chain ----- */

SwapChain* DbgRenderSystem::CreateSwapChain(const SwapChainDescriptor& swapChainDesc, const std::shared_ptr<Surface>& surface)
{
    /* Create swap-chain and flush frame profile on SwapChain::Present() calls  */
    return swapChains_.emplace<DbgSwapChain>(
        *instance_->CreateSwapChain(swapChainDesc, surface),
        swapChainDesc,
        std::bind(&DbgRenderSystem::FlushProfile, this)
    );
}

void DbgRenderSystem::Release(SwapChain& swapChain)
{
    ReleaseDbg(swapChains_, swapChain);
}

/* ----- Command queues ----- */

CommandQueue* DbgRenderSystem::GetCommandQueue()
{
    return commandQueue_.get();
}

/* ----- Command buffers ----- */

CommandBuffer* DbgRenderSystem::CreateCommandBuffer(const CommandBufferDescriptor& commandBufferDesc)
{
    ValidateCommandBufferDesc(commandBufferDesc);
    CommandBufferDescriptor instanceCommandBufferDesc;
    {
        instanceCommandBufferDesc.flags                 = commandBufferDesc.flags;
        instanceCommandBufferDesc.numNativeBuffers      = commandBufferDesc.numNativeBuffers;
        instanceCommandBufferDesc.minStagingPoolSize    = commandBufferDesc.minStagingPoolSize;
        instanceCommandBufferDesc.renderPass            = (commandBufferDesc.renderPass != nullptr
                                                        ? &(LLGL_CAST(const DbgRenderPass*, commandBufferDesc.renderPass)->instance)
                                                        : nullptr);
    }
    return commandBuffers_.emplace<DbgCommandBuffer>(
        *instance_,
        commandQueue_->instance,
        *instance_->CreateCommandBuffer(instanceCommandBufferDesc),
        profile_,
        debugger_,
        commandBufferDesc,
        GetRenderingCaps()
    );
}

void DbgRenderSystem::Release(CommandBuffer& commandBuffer)
{
    ReleaseDbg(commandBuffers_, commandBuffer);
}

/* ----- Buffers ------ */

Buffer* DbgRenderSystem::CreateBuffer(const BufferDescriptor& bufferDesc, const void* initialData)
{
    /* Validate and store format size (if supported) */
    std::uint32_t formatSize = 0;

    if (LLGL_DBG_SOURCE())
        ValidateBufferDesc(bufferDesc, &formatSize);

    /* Create buffer object */
    auto* bufferDbg = buffers_.emplace<DbgBuffer>(*instance_->CreateBuffer(bufferDesc, initialData), bufferDesc);
    {
        bufferDbg->elements     = (formatSize > 0 ? bufferDesc.size / formatSize : 0);
        bufferDbg->initialized  = (initialData != nullptr);
    }
    return bufferDbg;
}

BufferArray* DbgRenderSystem::CreateBufferArray(std::uint32_t numBuffers, Buffer* const * bufferArray)
{
    RenderSystem::AssertCreateBufferArray(numBuffers, bufferArray);

    /* Create temporary buffer array with buffer instances */
    std::vector<Buffer*>    bufferInstanceArray(numBuffers);
    std::vector<DbgBuffer*> bufferDbgArray(numBuffers);

    for (std::uint32_t i = 0; i < numBuffers; ++i)
    {
        auto* bufferDbg         = LLGL_CAST(DbgBuffer*, bufferArray[i]);
        bufferInstanceArray[i]  = &(bufferDbg->instance);
        bufferDbgArray[i]       = bufferDbg;
    }

    /* Create native buffer and debug buffer */
    auto* bufferArrayInstance = instance_->CreateBufferArray(numBuffers, bufferInstanceArray.data());
    return bufferArrays_.emplace<DbgBufferArray>(*bufferArrayInstance, GetCombinedBindFlags(numBuffers, bufferArray), std::move(bufferDbgArray));
}

void DbgRenderSystem::Release(Buffer& buffer)
{
    ReleaseDbg(buffers_, buffer);
}

void DbgRenderSystem::Release(BufferArray& bufferArray)
{
    ReleaseDbg(bufferArrays_, bufferArray);
}

void DbgRenderSystem::WriteBuffer(Buffer& buffer, std::uint64_t offset, const void* data, std::uint64_t dataSize)
{
    auto& bufferDbg = LLGL_CAST(DbgBuffer&, buffer);

    if (LLGL_DBG_SOURCE())
    {
        if (dataSize > 0)
        {
            /* Assume buffer to be initialized even if only partially as we cannot keep track of each bit inside the buffer */
            bufferDbg.initialized = true;
        }

        ValidateBufferBoundary(bufferDbg.desc.size, offset, dataSize);

        if (!data)
            LLGL_DBG_ERROR(ErrorType::InvalidArgument, "illegal null pointer argument for 'data' parameter");
    }

    instance_->WriteBuffer(bufferDbg.instance, offset, data, dataSize);

    profile_.commandQueueRecord.bufferWrites++;
}

void DbgRenderSystem::ReadBuffer(Buffer& buffer, std::uint64_t offset, void* data, std::uint64_t dataSize)
{
    auto& bufferDbg = LLGL_CAST(DbgBuffer&, buffer);

    if (LLGL_DBG_SOURCE())
    {
        if (!bufferDbg.initialized)
            LLGL_DBG_ERROR(ErrorType::InvalidState, "reading uninitialized buffer");
        if (!data)
            LLGL_DBG_ERROR(ErrorType::InvalidArgument, "illegal null pointer argument for 'data' parameter");

        ValidateBufferBoundary(bufferDbg.desc.size, offset, dataSize);
    }

    instance_->ReadBuffer(bufferDbg.instance, offset, data, dataSize);

    profile_.commandQueueRecord.bufferReads++;
}

void* DbgRenderSystem::MapBuffer(Buffer& buffer, const CPUAccess access)
{
    auto& bufferDbg = LLGL_CAST(DbgBuffer&, buffer);

    if (LLGL_DBG_SOURCE())
    {
        ValidateResourceCPUAccess(bufferDbg.desc.cpuAccessFlags, access, "buffer");
        ValidateBufferMapping(bufferDbg, true);
    }

    auto result = instance_->MapBuffer(bufferDbg.instance, access);

    if (result != nullptr)
        bufferDbg.OnMap(access, 0, bufferDbg.desc.size);

    profile_.commandQueueRecord.bufferMappings++;

    return result;
}

void* DbgRenderSystem::MapBuffer(Buffer& buffer, const CPUAccess access, std::uint64_t offset, std::uint64_t length)
{
    auto& bufferDbg = LLGL_CAST(DbgBuffer&, buffer);

    if (LLGL_DBG_SOURCE())
    {
        ValidateResourceCPUAccess(bufferDbg.desc.cpuAccessFlags, access, "buffer");
        ValidateBufferMapping(bufferDbg, true);
        ValidateBufferBoundary(bufferDbg.desc.size, offset, length);
    }

    auto result = instance_->MapBuffer(bufferDbg.instance, access, offset, length);

    if (result != nullptr)
        bufferDbg.OnMap(access, offset, length);

    profile_.commandQueueRecord.bufferMappings++;

    return result;
}

void DbgRenderSystem::UnmapBuffer(Buffer& buffer)
{
    auto& bufferDbg = LLGL_CAST(DbgBuffer&, buffer);

    if (LLGL_DBG_SOURCE())
        ValidateBufferMapping(bufferDbg, false);

    instance_->UnmapBuffer(bufferDbg.instance);

    bufferDbg.OnUnmap();
}

/* ----- Textures ----- */

Texture* DbgRenderSystem::CreateTexture(const TextureDescriptor& textureDesc, const ImageView* initialImage)
{
    if (LLGL_DBG_SOURCE())
        ValidateTextureDesc(textureDesc, initialImage);
    return textures_.emplace<DbgTexture>(*instance_->CreateTexture(textureDesc, initialImage), textureDesc);
}

void DbgRenderSystem::Release(Texture& texture)
{
    ReleaseDbg(textures_, texture);
}

void DbgRenderSystem::WriteTexture(Texture& texture, const TextureRegion& textureRegion, const ImageView& srcImageView)
{
    auto& textureDbg = LLGL_CAST(DbgTexture&, texture);

    if (LLGL_DBG_SOURCE())
    {
        ValidateTextureRegion(textureDbg, textureRegion);
        ValidateImageView(srcImageView, textureDbg.desc, &textureRegion);
    }

    instance_->WriteTexture(textureDbg.instance, textureRegion, srcImageView);

    profile_.commandQueueRecord.textureWrites++;
}

void DbgRenderSystem::ReadTexture(Texture& texture, const TextureRegion& textureRegion, const MutableImageView& dstImageView)
{
    auto& textureDbg = LLGL_CAST(DbgTexture&, texture);

    if (LLGL_DBG_SOURCE())
    {
        ValidateTextureRegion(textureDbg, textureRegion);
        ValidateImageView(ImageView{ dstImageView }, textureDbg.desc, &textureRegion);
    }

    instance_->ReadTexture(textureDbg.instance, textureRegion, dstImageView);

    profile_.commandQueueRecord.textureReads++;
}

/* ----- Sampler States ---- */

Sampler* DbgRenderSystem::CreateSampler(const SamplerDescriptor& samplerDesc)
{
    return instance_->CreateSampler(samplerDesc);
    //return samplers_.emplace<DbgSampler>();
}

void DbgRenderSystem::Release(Sampler& sampler)
{
    instance_->Release(sampler);
    //ReleaseDbg(samplers_, sampler);
}

/* ----- Resource Views ----- */

// private
std::vector<ResourceViewDescriptor> DbgRenderSystem::GetResourceViewInstanceCopy(const ArrayView<ResourceViewDescriptor>& resourceViews)
{
    std::vector<ResourceViewDescriptor> instanceResourceViews;
    instanceResourceViews.reserve(resourceViews.size());

    for_range(i, resourceViews.size())
    {
        ResourceViewDescriptor resourceViewCopy = resourceViews[i];
        if (Resource* resource = resourceViewCopy.resource)
        {
            switch (resource->GetResourceType())
            {
                case ResourceType::Buffer:
                    resourceViewCopy.resource = &(LLGL_CAST(DbgBuffer*, resourceViewCopy.resource)->instance);
                    break;
                case ResourceType::Texture:
                    resourceViewCopy.resource = &(LLGL_CAST(DbgTexture*, resourceViewCopy.resource)->instance);
                    break;
                case ResourceType::Sampler:
                    //TODO: DbgSampler
                    break;
                default:
                    LLGL_DBG_ERROR(ErrorType::InvalidArgument, "invalid resource type passed to <ResourceViewDescriptor>");
                    break;
            }
        }
        else
        {
            if (IsTextureViewEnabled(resourceViewCopy.textureView))
            {
                LLGL_DBG_WARN(
                    WarningType::ImproperArgument,
                    "texture view is enabled in ResourceViewDescriptor[%zu] but resource is null",
                    i
                );
            }
            if (IsBufferViewEnabled(resourceViewCopy.bufferView))
            {
                LLGL_DBG_WARN(
                    WarningType::ImproperArgument,
                    "buffer view is enabled in ResourceViewDescriptor[%zu] but resource is null",
                    i
                );
            }
        }
        instanceResourceViews.push_back(resourceViewCopy);
    }

    return instanceResourceViews;
}

ResourceHeap* DbgRenderSystem::CreateResourceHeap(const ResourceHeapDescriptor& resourceHeapDesc, const ArrayView<ResourceViewDescriptor>& initialResourceViews)
{
    if (LLGL_DBG_SOURCE())
        ValidateResourceHeapDesc(resourceHeapDesc, initialResourceViews);

    /* Create copy of resource view descriptors to pass native resource object references */
    auto instanceResourceViews = GetResourceViewInstanceCopy(initialResourceViews);

    /* Create copy of descriptor to pass native renderer object references */
    auto instanceDesc = resourceHeapDesc;
    {
        auto pipelineLayoutDbg = LLGL_CAST(DbgPipelineLayout*, resourceHeapDesc.pipelineLayout);
        instanceDesc.pipelineLayout = &(pipelineLayoutDbg->instance);
    }
    return resourceHeaps_.emplace<DbgResourceHeap>(
        *instance_->CreateResourceHeap(instanceDesc, instanceResourceViews),
        resourceHeapDesc
    );
}

void DbgRenderSystem::Release(ResourceHeap& resourceHeap)
{
    ReleaseDbg(resourceHeaps_, resourceHeap);
}

std::uint32_t DbgRenderSystem::WriteResourceHeap(ResourceHeap& resourceHeap, std::uint32_t firstDescriptor, const ArrayView<ResourceViewDescriptor>& resourceViews)
{
    auto& resourceHeapDbg = LLGL_CAST(DbgResourceHeap&, resourceHeap);

    if (LLGL_DBG_SOURCE())
        ValidateResourceHeapRange(resourceHeapDbg, firstDescriptor, resourceViews);

    auto instanceResourceViews = GetResourceViewInstanceCopy(resourceViews);
    return instance_->WriteResourceHeap(resourceHeapDbg.instance, firstDescriptor, instanceResourceViews);
}

/* ----- Render Passes ----- */

RenderPass* DbgRenderSystem::CreateRenderPass(const RenderPassDescriptor& renderPassDesc)
{
    return renderPasses_.emplace<DbgRenderPass>(*instance_->CreateRenderPass(renderPassDesc), renderPassDesc);
}

void DbgRenderSystem::Release(RenderPass& renderPass)
{
    /* Render passes have to be deleted manually with an explicitly multable instance, because they can be queried from RenderTarget::GetRenderPass() */
    auto& renderPassDbg = LLGL_CAST(DbgRenderPass&, renderPass);
    if (RenderPass* instance = renderPassDbg.mutableInstance)
    {
        instance_->Release(*instance);
        renderPasses_.erase(&renderPass);
    }
}

/* ----- Render Targets ----- */

RenderTarget* DbgRenderSystem::CreateRenderTarget(const RenderTargetDescriptor& renderTargetDesc)
{
    LLGL_DBG_SOURCE();

    RenderTargetDescriptor instanceDesc = renderTargetDesc;
    {
        instanceDesc.renderPass = DbgGetInstance<DbgRenderPass>(renderTargetDesc.renderPass);

        auto TransferDbgAttachment = [this](AttachmentDescriptor& attachmentDesc, std::uint32_t colorTarget, bool isResolveAttachment, bool isDepthStencilAttachment)
        {
            if (IsAttachmentEnabled(attachmentDesc))
            {
                if (debugger_)
                    ValidateAttachmentDesc(attachmentDesc, colorTarget, isResolveAttachment, isDepthStencilAttachment);
                attachmentDesc.texture = DbgGetInstance<DbgTexture>(attachmentDesc.texture);
            }
        };

        for_range(colorTarget, LLGL_MAX_NUM_COLOR_ATTACHMENTS)
        {
            TransferDbgAttachment(instanceDesc.colorAttachments[colorTarget], colorTarget, /*isResolveAttachment:*/ false, /*isDepthStencilAttachment:*/ false);
            TransferDbgAttachment(instanceDesc.resolveAttachments[colorTarget], colorTarget, /*isResolveAttachment:*/ true, /*isDepthStencilAttachment:*/ false);
        }
        TransferDbgAttachment(instanceDesc.depthStencilAttachment, 0, /*isResolveAttachment:*/ false, /*isDepthStencilAttachment:*/ true);
    }
    return renderTargets_.emplace<DbgRenderTarget>(*instance_->CreateRenderTarget(instanceDesc), renderTargetDesc);
}

void DbgRenderSystem::Release(RenderTarget& renderTarget)
{
    ReleaseDbg(renderTargets_, renderTarget);
}

/* ----- Shader ----- */

Shader* DbgRenderSystem::CreateShader(const ShaderDescriptor& shaderDesc)
{
    if (LLGL_DBG_SOURCE())
        ValidateShaderDesc(shaderDesc);
    return shaders_.emplace<DbgShader>(*instance_->CreateShader(shaderDesc), shaderDesc);
}

void DbgRenderSystem::Release(Shader& shader)
{
    ReleaseDbg(shaders_, shader);
}

/* ----- Pipeline Layouts ----- */

PipelineLayout* DbgRenderSystem::CreatePipelineLayout(const PipelineLayoutDescriptor& pipelineLayoutDesc)
{
    if (LLGL_DBG_SOURCE())
        ValidatePipelineLayoutDesc(pipelineLayoutDesc);
    return pipelineLayouts_.emplace<DbgPipelineLayout>(*instance_->CreatePipelineLayout(pipelineLayoutDesc), pipelineLayoutDesc);
}

void DbgRenderSystem::Release(PipelineLayout& pipelineLayout)
{
    ReleaseDbg(pipelineLayouts_, pipelineLayout);
}

/* ----- Pipeline Caches ----- */

PipelineCache* DbgRenderSystem::CreatePipelineCache(const Blob& initialBlob)
{
    return instance_->CreatePipelineCache(initialBlob);
}

void DbgRenderSystem::Release(PipelineCache& pipelineCache)
{
    instance_->Release(pipelineCache);
}

/* ----- Pipeline States ----- */

PipelineState* DbgRenderSystem::CreatePipelineState(const GraphicsPipelineDescriptor& pipelineStateDesc, PipelineCache* pipelineCache)
{
    if (LLGL_DBG_SOURCE())
        ValidateGraphicsPipelineDesc(pipelineStateDesc);

    GraphicsPipelineDescriptor instanceDesc = pipelineStateDesc;
    {
        if (pipelineStateDesc.pipelineLayout != nullptr)
            instanceDesc.pipelineLayout = &(LLGL_CAST(const DbgPipelineLayout*, pipelineStateDesc.pipelineLayout)->instance);

        instanceDesc.renderPass             = DbgGetInstance<DbgRenderPass>(pipelineStateDesc.renderPass);
        instanceDesc.vertexShader           = DbgGetInstance<DbgShader>(pipelineStateDesc.vertexShader);
        instanceDesc.tessControlShader      = DbgGetInstance<DbgShader>(pipelineStateDesc.tessControlShader);
        instanceDesc.tessEvaluationShader   = DbgGetInstance<DbgShader>(pipelineStateDesc.tessEvaluationShader);
        instanceDesc.geometryShader         = DbgGetInstance<DbgShader>(pipelineStateDesc.geometryShader);
        instanceDesc.fragmentShader         = DbgGetInstance<DbgShader>(pipelineStateDesc.fragmentShader);
    }
    return pipelineStates_.emplace<DbgPipelineState>(*instance_->CreatePipelineState(instanceDesc, pipelineCache), pipelineStateDesc);
}

PipelineState* DbgRenderSystem::CreatePipelineState(const ComputePipelineDescriptor& pipelineStateDesc, PipelineCache* pipelineCache)
{
    if (LLGL_DBG_SOURCE())
        ValidateComputePipelineDesc(pipelineStateDesc);

    ComputePipelineDescriptor instanceDesc = pipelineStateDesc;
    {
        if (pipelineStateDesc.pipelineLayout != nullptr)
            instanceDesc.pipelineLayout = &(LLGL_CAST(const DbgPipelineLayout*, pipelineStateDesc.pipelineLayout)->instance);

        instanceDesc.computeShader = DbgGetInstance<DbgShader>(pipelineStateDesc.computeShader);
    }
    return pipelineStates_.emplace<DbgPipelineState>(*instance_->CreatePipelineState(instanceDesc, pipelineCache), pipelineStateDesc);
}

void DbgRenderSystem::Release(PipelineState& pipelineState)
{
    ReleaseDbg(pipelineStates_, pipelineState);
}

/* ----- Queries ----- */

QueryHeap* DbgRenderSystem::CreateQueryHeap(const QueryHeapDescriptor& queryHeapDesc)
{
    return queryHeaps_.emplace<DbgQueryHeap>(*instance_->CreateQueryHeap(queryHeapDesc), queryHeapDesc);
}

void DbgRenderSystem::Release(QueryHeap& queryHeap)
{
    ReleaseDbg(queryHeaps_, queryHeap);
}

/* ----- Fences ----- */

Fence* DbgRenderSystem::CreateFence()
{
    return instance_->CreateFence();
}

void DbgRenderSystem::Release(Fence& fence)
{
    instance_->Release(fence);
}

/* ----- Extensions ----- */

bool DbgRenderSystem::GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize)
{
    return instance_->GetNativeHandle(nativeHandle, nativeHandleSize);
}


/*
 * ======= Private: =======
 */

bool DbgRenderSystem::QueryRendererDetails(RendererInfo* outInfo, RenderingCapabilities* outCaps)
{
    if (outInfo != nullptr)
        *outInfo = instance_->GetRendererInfo();
    if (outCaps != nullptr)
        *outCaps = instance_->GetRenderingCaps();
    return true;
}

void DbgRenderSystem::ValidateBindFlags(long flags, Format format, ResourceType resourceType)
{
    constexpr long bufferOnlyFlags =
    (
        BindFlags::VertexBuffer         |
        BindFlags::IndexBuffer          |
        BindFlags::ConstantBuffer       |
        BindFlags::StreamOutputBuffer   |
        BindFlags::IndirectBuffer
    );

    constexpr long textureOnlyFlags =
    (
        BindFlags::ColorAttachment          |
        BindFlags::DepthStencilAttachment
    );

    constexpr long validFlags =
    (
        bufferOnlyFlags     |
        textureOnlyFlags    |
        BindFlags::Sampled  |
        BindFlags::Storage  |
        BindFlags::CopySrc  |
        BindFlags::CopyDst
    );

    constexpr long cbufferExcludedFlags =
    (
        BindFlags::VertexBuffer         |
        BindFlags::IndexBuffer          |
        BindFlags::StreamOutputBuffer   |
        BindFlags::IndirectBuffer       |
        BindFlags::Sampled              |
        BindFlags::Storage
    );

    /* Check for unknown flags */
    if ((flags & (~validFlags)) != 0)
        LLGL_DBG_WARN(WarningType::ImproperArgument, "unknown bind flags specified");

    /* Validate combination of flags */
    if (resourceType != ResourceType::Undefined)
    {
        if ((flags & bufferOnlyFlags) != 0)
        {
            if (resourceType != ResourceType::Buffer)
            {
                LLGL_DBG_ERROR(
                    ErrorType::InvalidArgument,
                    "cannot use buffer-only bind flags for %s source type",
                    ToString(resourceType)
                );
            }
        }
        if ((flags & textureOnlyFlags) != 0)
        {
            if (resourceType != ResourceType::Texture)
            {
                LLGL_DBG_ERROR(
                    ErrorType::InvalidArgument,
                    "cannot use texture-only bind flags for %s source type",
                    ToString(resourceType)
                );
            }
        }
    }
    else if ((flags & bufferOnlyFlags) != 0 && (flags & textureOnlyFlags) != 0)
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "cannot combine binding flags that are exclusive for buffers and textures"
        );
    }

    if ((flags & BindFlags::ColorAttachment) != 0 && (flags & BindFlags::DepthStencilAttachment) != 0)
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "resources cannot have color attachment and depth-stencil attachment binding flags at the same time"
        );
    }
    else if (format != Format::Undefined && resourceType == ResourceType::Texture)
    {
        if ((flags & BindFlags::ColorAttachment) != 0)
        {
            if (!IsColorFormat(format))
            {
                LLGL_DBG_ERROR(
                    ErrorType::InvalidArgument,
                    "cannot use bind flag LLGL::BindFlags::ColorAttachment for texture with non-color format (%s)",
                    ToString(format)
                );
            }
        }
        else if ((flags & BindFlags::DepthStencilAttachment) != 0)
        {
            if (!IsDepthOrStencilFormat(format))
            {
                LLGL_DBG_ERROR(
                    ErrorType::InvalidArgument,
                    "cannot use bind flag LLGL::BindFlags::DepthStencilAttachment for texture with non-depth-stencil format (%s)",
                    ToString(format)
                );
            }
        }
    }

    if ((flags & BindFlags::ConstantBuffer) != 0 && (flags & cbufferExcludedFlags) != 0)
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "cannot combine bind flag LLGL::BindFlags::ConstantBuffer with any other bind flag except LLGL::BindFlags::CopySrc and LLGL::BindFlags::CopyDst"
        );
    }
}

void DbgRenderSystem::ValidateCPUAccessFlags(long flags, long validFlags, const char* contextDesc)
{
    if ((flags & (~validFlags)) != 0)
    {
        const std::string contextLabel = (contextDesc != nullptr ? " for " + std::string(contextDesc) : "");
        LLGL_DBG_WARN(WarningType::ImproperArgument, "unknown CPU access flags specified%s", contextLabel.c_str());
    }
}

void DbgRenderSystem::ValidateMiscFlags(long flags, long validFlags, const char* contextDesc)
{
    if ((flags & (~validFlags)) != 0)
    {
        const std::string contextLabel = (contextDesc != nullptr ? " for " + std::string(contextDesc) : "");
        LLGL_DBG_WARN(WarningType::ImproperArgument, "unknown miscellaneous flags specified%s", contextLabel.c_str());
    }
}

void DbgRenderSystem::ValidateResourceCPUAccess(long cpuAccessFlags, const CPUAccess access, const char* resourceTypeName)
{
    if (access == CPUAccess::ReadOnly || access == CPUAccess::ReadWrite)
    {
        if ((cpuAccessFlags & CPUAccessFlags::Read) == 0)
        {
            LLGL_DBG_ERROR(
                ErrorType::InvalidState,
                "cannot map %s with CPU read access, because the resource was not created with 'LLGL::CPUAccessFlags::Read' flag",
                resourceTypeName
            );
        }
    }
    if (access == CPUAccess::WriteOnly || access == CPUAccess::ReadWrite)
    {
        if ((cpuAccessFlags & CPUAccessFlags::Write) == 0)
        {
            LLGL_DBG_ERROR(
                ErrorType::InvalidState,
                "cannot map %s with CPU write access, because the resource was not created with 'LLGL::CPUAccessFlags::Write' flag",
                resourceTypeName
            );
        }
    }
}

void DbgRenderSystem::ValidateCommandBufferDesc(const CommandBufferDescriptor& commandBufferDesc)
{
    /* Validate flags */
    if ((commandBufferDesc.flags & CommandBufferFlags::ImmediateSubmit) != 0)
    {
        if ((commandBufferDesc.flags & (CommandBufferFlags::Secondary | CommandBufferFlags::MultiSubmit)) != 0)
            LLGL_DBG_ERROR(ErrorType::InvalidArgument, "cannot create immediate command buffer with Secondary or MultiSubmit flags");
    }
    if (commandBufferDesc.renderPass != nullptr)
    {
        if ((commandBufferDesc.flags & CommandBufferFlags::Secondary) == 0)
            LLGL_DBG_WARN(WarningType::ImproperArgument, "render pass is ignored for primary command buffers at creation time");
    }
}

void DbgRenderSystem::ValidateBufferDesc(const BufferDescriptor& bufferDesc, std::uint32_t* formatSizeOut)
{
    /* Validate flags */
    ValidateBindFlags(bufferDesc.bindFlags, bufferDesc.format, ResourceType::Buffer);
    ValidateCPUAccessFlags(bufferDesc.cpuAccessFlags, CPUAccessFlags::ReadWrite, "buffer");
    ValidateMiscFlags(bufferDesc.miscFlags, (MiscFlags::DynamicUsage | MiscFlags::NoInitialData), "buffer");

    /* Validate (constant-) buffer size */
    if ((bufferDesc.bindFlags & BindFlags::ConstantBuffer) != 0)
        ValidateConstantBufferSize(bufferDesc.size);
    else
        ValidateBufferSize(bufferDesc.size);

    std::uint32_t formatSize = 0;

    if ((bufferDesc.bindFlags & BindFlags::VertexBuffer) != 0 && !bufferDesc.vertexAttribs.empty())
    {
        /* Validate all vertex attributes have the same binding slot */
        if (bufferDesc.vertexAttribs.size() >= 2)
        {
            for (std::size_t i = 0; i + 1 < bufferDesc.vertexAttribs.size(); ++i)
                ValidateVertexAttributesForBuffer(bufferDesc.vertexAttribs[i], bufferDesc.vertexAttribs[i + 1]);
        }

        /* Validate buffer size for specified vertex format, unless it's also used for as index buffer */
        formatSize = bufferDesc.vertexAttribs.front().stride;
        if (formatSize > 0 && bufferDesc.size % formatSize != 0 && (bufferDesc.bindFlags & BindFlags::IndexBuffer) == 0)
            LLGL_DBG_WARN(WarningType::ImproperArgument, "improper vertex buffer size with vertex format of %u %s", formatSize, ToByteLabel(formatSize));
    }

    if ((bufferDesc.bindFlags & BindFlags::IndexBuffer) != 0 && bufferDesc.format != Format::Undefined)
    {
        /* Validate index format */
        if (bufferDesc.format != Format::R16UInt &&
            bufferDesc.format != Format::R32UInt)
        {
            if (const char* formatName = ToString(bufferDesc.format))
                LLGL_DBG_ERROR(ErrorType::InvalidArgument, "invalid index buffer format: LLGL::Format::%s", formatName);
            else
                LLGL_DBG_ERROR(ErrorType::InvalidArgument, "unknown index buffer format: 0x%08X", static_cast<unsigned>(bufferDesc.format));
        }

        /* Validate buffer size for specified index format, unless it's also used for as vertex buffer  */
        formatSize = GetFormatAttribs(bufferDesc.format).bitSize / 8;
        if (formatSize > 0 && bufferDesc.size % formatSize != 0 && (bufferDesc.bindFlags & BindFlags::VertexBuffer) == 0)
        {
            LLGL_DBG_WARN(
                WarningType::ImproperArgument,
                "improper index buffer size with index format of %u %s", formatSize, ToByteLabel(formatSize)
            );
        }
    }

    if ((bufferDesc.bindFlags & BindFlags::ConstantBuffer) != 0)
    {
        /* Validate pack alginemnt of 16 bytes */
        static const std::uint64_t packAlignment = 16;
        if (bufferDesc.size % packAlignment != 0)
        {
            LLGL_DBG_WARN(
                WarningType::ImproperArgument,
                "constant buffer size (%" PRIu64 ") is out of pack alignment (%" PRIu64 ")",
                bufferDesc.size, packAlignment
            );
        }
    }

    /* Validate buffer stride */
    if (bufferDesc.stride > 0 && bufferDesc.size % bufferDesc.stride != 0)
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "buffer stride (%u) is non-zero, but buffer size (%" PRIu64 ") is not a multiple of stride",
            bufferDesc.stride, bufferDesc.size
        );
    }

    if (formatSizeOut)
        *formatSizeOut = formatSize;
}

void DbgRenderSystem::ValidateVertexAttributesForBuffer(const VertexAttribute& lhs, const VertexAttribute& rhs)
{
    if (lhs.slot != rhs.slot || lhs.stride != rhs.stride || lhs.instanceDivisor != rhs.instanceDivisor)
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "vertex attributes must have equal slot, stride, and instance divisor within the same buffer, but found mismatch between \"%s\" and \"%s\"",
            lhs.name.c_str(), rhs.name.c_str()
        );
    }
}

void DbgRenderSystem::ValidateBufferSize(std::uint64_t size)
{
    const RenderingLimits& limits = GetRenderingCaps().limits;
    if (size > limits.maxBufferSize)
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "buffer size exceeded limit: %" PRIu64 " specified but limit is %" PRIu64,
            size, limits.maxBufferSize
        );
    }
}

void DbgRenderSystem::ValidateConstantBufferSize(std::uint64_t size)
{
    const RenderingLimits& limits = GetRenderingCaps().limits;
    if (size > limits.maxConstantBufferSize)
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "constant buffer size exceeded limit: %" PRIu64 " specified but limit is %" PRIu64,
            size, limits.maxConstantBufferSize
        );
    }
}

void DbgRenderSystem::ValidateBufferBoundary(std::uint64_t bufferSize, std::uint64_t dstOffset, std::uint64_t dataSize)
{
    if (dstOffset >= bufferSize)
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "buffer offset out of bounds: %" PRIu64 " specified but upper bound is %" PRIu64,
            dstOffset, bufferSize
        );
    }
    else if (dataSize + dstOffset > bufferSize)
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "data size for buffer offset out of bounds: %" PRIu64 "+%" PRIu64 " specified but limit is %" PRIu64,
            dstOffset, dataSize, bufferSize
        );
    }
}

void DbgRenderSystem::ValidateBufferMapping(DbgBuffer& bufferDbg, bool mapMemory)
{
    if (mapMemory)
    {
        if (bufferDbg.IsMappedForCPUAccess())
            LLGL_DBG_ERROR(ErrorType::InvalidState, "cannot map buffer that has already been mapped to CPU memory space");
    }
    else
    {
        if (!bufferDbg.IsMappedForCPUAccess())
            LLGL_DBG_ERROR(ErrorType::InvalidState, "cannot unmap buffer that was not previously mapped to CPU memory space");
    }
}

static std::uint64_t GetMinAlignmentForBufferBinding(const BindingDescriptor& binding, const RenderingLimits& limits)
{
    std::uint64_t alignment = 0;
    if ((binding.bindFlags & BindFlags::ConstantBuffer) != 0)
        alignment = limits.minConstantBufferAlignment;
    if ((binding.bindFlags & BindFlags::Sampled) != 0)
        alignment = (std::max)(alignment, limits.minSampledBufferAlignment);
    if ((binding.bindFlags & BindFlags::Storage) != 0)
        alignment = (std::max)(alignment, limits.minStorageBufferAlignment);
    return alignment;
}

void DbgRenderSystem::ValidateBufferView(DbgBuffer& bufferDbg, const BufferViewDescriptor& viewDesc, const BindingDescriptor& bindingDesc)
{
    const RenderingLimits& limits = GetRenderingCaps().limits;
    const std::uint64_t minAlignment = GetMinAlignmentForBufferBinding(bindingDesc, limits);
    if (minAlignment > 0 && (viewDesc.offset % minAlignment != 0 || viewDesc.size % minAlignment != 0))
    {
        const std::string bindingSetLabel = (bindingDesc.slot.set != 0 ? "(set " + std::to_string(bindingDesc.slot.set) + ')' : "");
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "buffer view '%s' at slot %u%s does not satisfy minimum alignment of %" PRIu64 " %s",
            bindingDesc.name.c_str(), bindingDesc.slot.index, bindingSetLabel.c_str(), minAlignment, ToByteLabel(minAlignment)
        );
    }
}

void DbgRenderSystem::ValidateTextureDesc(const TextureDescriptor& textureDesc, const ImageView* initialImage)
{
    switch (textureDesc.type)
    {
        case TextureType::Texture1D:
            Validate1DTextureSize(textureDesc.extent.width);
            ValidateTextureSizePassiveDimension(textureDesc.extent.height, "1D", "Y");
            ValidateTextureSizePassiveDimension(textureDesc.extent.depth, "1D", "Z");
            break;

        case TextureType::Texture2D:
            Validate2DTextureSize(textureDesc.extent.width);
            Validate2DTextureSize(textureDesc.extent.height);
            ValidateTextureSizePassiveDimension(textureDesc.extent.depth, "2D", "Z");
            break;

        case TextureType::TextureCube:
            AssertCubeTextures();
            ValidateCubeTextureSize(textureDesc.extent.width, textureDesc.extent.height);
            ValidateTextureSizePassiveDimension(textureDesc.extent.depth, "cube", "Z");
            break;

        case TextureType::Texture3D:
            Assert3DTextures();
            Validate3DTextureSize(textureDesc.extent.width);
            Validate3DTextureSize(textureDesc.extent.height);
            Validate3DTextureSize(textureDesc.extent.depth);
            break;

        case TextureType::Texture1DArray:
            AssertArrayTextures();
            Validate1DTextureSize(textureDesc.extent.width);
            ValidateTextureSizePassiveDimension(textureDesc.extent.height, "1D-array", "Y");
            ValidateTextureSizePassiveDimension(textureDesc.extent.depth, "1D-array", "Z");
            break;

        case TextureType::Texture2DArray:
            AssertArrayTextures();
            Validate1DTextureSize(textureDesc.extent.width);
            Validate1DTextureSize(textureDesc.extent.height);
            ValidateTextureSizePassiveDimension(textureDesc.extent.depth, "2D-array", "Z");
            break;

        case TextureType::TextureCubeArray:
            AssertCubeArrayTextures();
            ValidateCubeTextureSize(textureDesc.extent.width, textureDesc.extent.height);
            ValidateTextureSizePassiveDimension(textureDesc.extent.depth, "cube-array", "Z");
            break;

        case TextureType::Texture2DMS:
            AssertMultiSampleTextures();
            Validate2DTextureSize(textureDesc.extent.width);
            Validate2DTextureSize(textureDesc.extent.height);
            ValidateTextureSizePassiveDimension(textureDesc.extent.depth, "2DMS", "Z");
            break;

        case TextureType::Texture2DMSArray:
            AssertMultiSampleTextures();
            AssertArrayTextures();
            Validate2DTextureSize(textureDesc.extent.width);
            Validate2DTextureSize(textureDesc.extent.height);
            ValidateTextureSizePassiveDimension(textureDesc.extent.depth, "2DMS-array", "Z");
            break;

        default:
            LLGL_DBG_ERROR(ErrorType::InvalidArgument, "invalid texture type");
            break;
    }

    ValidateTextureFormatSupported(textureDesc.format);
    ValidateTextureDescMipLevels(textureDesc);
    ValidateArrayTextureLayers(textureDesc.type, textureDesc.arrayLayers);
    ValidateBindFlags(textureDesc.bindFlags, textureDesc.format, ResourceType::Texture);
    ValidateMiscFlags(textureDesc.miscFlags, (MiscFlags::DynamicUsage | MiscFlags::FixedSamples | MiscFlags::GenerateMips | MiscFlags::NoInitialData), "texture");

    if (initialImage != nullptr)
        ValidateImageView(*initialImage, textureDesc);

    /* Check if MIP-map generation is requested  */
    if ((textureDesc.miscFlags & MiscFlags::GenerateMips) != 0)
    {
        if (initialImage == nullptr)
        {
            #if 0
            LLGL_DBG_WARN(
                WarningType::ImproperArgument,
                "cannot generate MIP-maps without initial image data: 'LLGL::MiscFlags::GenerateMips' specified but no initial image data"
            );
            #endif
        }
        else if ((textureDesc.miscFlags & MiscFlags::NoInitialData) != 0)
        {
            LLGL_DBG_WARN(
                WarningType::ImproperArgument,
                "cannot generate MIP-maps with initial image data discarded: 'LLGL::MiscFlags::GenerateMips' specified but also 'MiscFlags::NoInitialData'"
            );
        }
    }
}

void DbgRenderSystem::ValidateTextureFormatSupported(const Format format)
{
    const auto& supportedFormats = GetRenderingCaps().textureFormats;
    if (std::find(supportedFormats.begin(), supportedFormats.end(), format) == supportedFormats.end())
    {
        LLGL_DBG_ERROR(
            ErrorType::UnsupportedFeature,
            "cannot create texture with unsupported format: %s", ToString(format)
        );
    }
}

void DbgRenderSystem::ValidateTextureDescMipLevels(const TextureDescriptor& textureDesc)
{
    if (textureDesc.mipLevels > 1)
    {
        /* Get number of levels for full MIP-chain */
        auto tempDesc = textureDesc;
        tempDesc.mipLevels = 0;
        auto maxNumMipLevels = NumMipLevels(tempDesc);

        if (textureDesc.mipLevels > maxNumMipLevels)
        {
            LLGL_DBG_ERROR(
                ErrorType::InvalidArgument,
                "number of MIP-map levels exceeded limit: %u specified but limit is %u",
                textureDesc.mipLevels, maxNumMipLevels
            );
        }
    }
}

void DbgRenderSystem::ValidateTextureSize(std::uint32_t size, std::uint32_t limit, const char* textureTypeName)
{
    if (size == 0)
        LLGL_DBG_ERROR(ErrorType::InvalidArgument, "texture size must not be 0");
    if (size > limit)
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "%s texture size exceeded limit: %u specified but limit is %u",
            textureTypeName, size, limit
        );
    }
}

void DbgRenderSystem::ValidateTextureSizePassiveDimension(std::uint32_t size, const char* textureTypeName, const char* axisName)
{
    if (size == 0)
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "%s texture size for %s-axis must not be 0",
            textureTypeName, axisName
        );
    }
    if (size > 1)
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "%s texture size of %s-axis must be 1, but %u was specified",
            textureTypeName, axisName, size
        );
    }
}

void DbgRenderSystem::Validate1DTextureSize(std::uint32_t size)
{
    const RenderingLimits& limits = GetRenderingCaps().limits;
    ValidateTextureSize(size, limits.max1DTextureSize, "1D");
}

void DbgRenderSystem::Validate2DTextureSize(std::uint32_t size)
{
    const RenderingLimits& limits = GetRenderingCaps().limits;
    ValidateTextureSize(size, limits.max2DTextureSize, "2D");
}

void DbgRenderSystem::Validate3DTextureSize(std::uint32_t size)
{
    const RenderingLimits& limits = GetRenderingCaps().limits;
    ValidateTextureSize(size, limits.max3DTextureSize, "3D");
}

void DbgRenderSystem::ValidateCubeTextureSize(std::uint32_t width, std::uint32_t height)
{
    const RenderingLimits& limits = GetRenderingCaps().limits;
    ValidateTextureSize(width, limits.maxCubeTextureSize, "cube");
    ValidateTextureSize(height, limits.maxCubeTextureSize, "cube");
    if (width != height)
        LLGL_DBG_ERROR(ErrorType::InvalidArgument, "width and height of cube textures must be equal");
}

void DbgRenderSystem::ValidateArrayTextureLayers(const TextureType type, std::uint32_t layers)
{
    if (layers == 0)
        LLGL_DBG_ERROR(ErrorType::InvalidArgument, "number of texture array layers must not be 0");

    if (layers > 1)
    {
        switch (type)
        {
            case TextureType::TextureCube:
            {
                if (layers != 6)
                {
                    LLGL_DBG_ERROR(
                        ErrorType::InvalidArgument,
                        "number of texture layers must be 6 for cube textures, but %u was specified",
                        layers
                    );
                }
            }
            break;

            case TextureType::TextureCubeArray:
            {
                if (layers % 6 != 0)
                {
                    LLGL_DBG_ERROR(
                        ErrorType::InvalidArgument,
                        "number of texture layers must be a multiple of 6 for cube array textures, but %u was specified",
                        layers
                    );
                }
            }
            break;

            default:
            {
                if (IsArrayTexture(type))
                {
                    const RenderingLimits& limits = GetRenderingCaps().limits;
                    const std::uint32_t maxNumLayers = limits.maxTextureArrayLayers;
                    if (layers > maxNumLayers)
                    {
                        LLGL_DBG_ERROR(
                            ErrorType::InvalidArgument,
                            "number of texture layers exceeded limit: %u specified but limit is %u",
                            layers, maxNumLayers
                        );
                    }
                }
                else
                {
                    LLGL_DBG_ERROR(
                        ErrorType::InvalidArgument,
                        "number of texture array layers must be 1 for non-array textures, but %u was specified",
                        layers
                    );
                }
            }
            break;
        }
    }
}

void DbgRenderSystem::ValidateMipLevelLimit(std::uint32_t baseMipLevel, std::uint32_t numMipLevels, std::uint32_t maxNumMipLevels)
{
    if (baseMipLevel + numMipLevels > maxNumMipLevels)
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "mip level out of bounds: %u exceeded limit of %u",
            (baseMipLevel + numMipLevels), numMipLevels
        );
    }
}

static std::uint32_t GetTextureSubresourceNumTexels(const TextureDescriptor& textureDesc, const TextureRegion* textureRegion)
{
    if (textureRegion != nullptr)
    {
        const TextureSubresource&   subresource     = textureRegion->subresource;
        const TextureSubresource    baseSubresource = TextureSubresource{ 0, subresource.numArrayLayers, 0, subresource.numMipLevels };
        return NumMipTexels(textureDesc.type, textureRegion->extent, baseSubresource);
    }
    return NumMipTexels(textureDesc, 0);
}

//TODO: also support compressed formats in validation
void DbgRenderSystem::ValidateImageView(const ImageView& imageView, const TextureDescriptor& textureDesc, const TextureRegion* textureRegion)
{
    /* Validate output data size */
    const std::uint32_t numTexels           = GetTextureSubresourceNumTexels(textureDesc, textureRegion);
    const std::size_t   requiredDataSize    = GetMemoryFootprint(imageView.format, imageView.dataType, numTexels);

    /* Ignore compressed formats */
    if (requiredDataSize != 0)
    {
        if (imageView.dataSize < requiredDataSize)
        {
            LLGL_DBG_ERROR(
                ErrorType::InvalidArgument,
                "image data size too small for texture: %zu %s specified but required is %zu %s",
                imageView.dataSize, ToByteLabel(imageView.dataSize), requiredDataSize, ToByteLabel(requiredDataSize)
            );
        }
        else if (imageView.dataSize > requiredDataSize)
        {
            LLGL_DBG_WARN(
                WarningType::ImproperArgument,
                "image data size larger than expected for texture: %zu %s specified but required is %zu %s",
                imageView.dataSize, ToByteLabel(imageView.dataSize), requiredDataSize, ToByteLabel(requiredDataSize)
            );
        }
    }

    /* Validate row and layer stride */
    if (imageView.rowStride != 0)
    {
        if (imageView.layerStride != 0)
        {
            if (imageView.layerStride % imageView.rowStride != 0)
            {
                LLGL_DBG_ERROR(
                    ErrorType::InvalidArgument,
                    "image layer-stride (%u) is not a multiple of row-stride (%u)",
                    imageView.layerStride, imageView.rowStride
                );
            }
        }
    }
    else if (imageView.layerStride != 0)
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "image layuer-stride (%u) is non-zero, but row-stride is zero",
            imageView.layerStride
        );
    }
}

void DbgRenderSystem::ValidateTextureArrayRange(const DbgTexture& textureDbg, std::uint32_t baseArrayLayer, std::uint32_t numArrayLayers)
{
    if (IsArrayTexture(textureDbg.GetType()))
        ValidateTextureArrayRangeWithEnd(baseArrayLayer, numArrayLayers, textureDbg.desc.arrayLayers);
    else if (baseArrayLayer > 0 || numArrayLayers > 1)
        LLGL_DBG_ERROR(ErrorType::InvalidArgument, "array layer out of range for non-array texture type");
}

void DbgRenderSystem::ValidateTextureArrayRangeWithEnd(std::uint32_t baseArrayLayer, std::uint32_t numArrayLayers, std::uint32_t arrayLayerLimit)
{
    const std::uint32_t arrayLayerRangeEnd = baseArrayLayer + numArrayLayers;
    if (arrayLayerRangeEnd > arrayLayerLimit)
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "array layer out of range for array texture: %u specified but limit is %u",
            arrayLayerRangeEnd, arrayLayerLimit
        );
    }
}

void DbgRenderSystem::ValidateTextureRegion(const DbgTexture& textureDbg, const TextureRegion& textureRegion)
{
    /* Validate MIP-map level range */
    ValidateMipLevelLimit(
        textureRegion.subresource.baseMipLevel,
        textureRegion.subresource.numMipLevels,
        textureDbg.mipLevels
    );

    /* Validate array layer range */
    ValidateTextureArrayRangeWithEnd(
        textureRegion.subresource.baseArrayLayer,
        textureRegion.subresource.numArrayLayers,
        textureDbg.desc.arrayLayers
    );

    /* Validate offset */
    if (textureRegion.offset.x < 0 || textureRegion.offset.y < 0 || textureRegion.offset.z < 0)
        LLGL_DBG_ERROR(ErrorType::UndefinedBehavior, "negative offset not allowed to write a texture region");

    /* Validate offset plus extent */
    auto IsRegionOutside = [](std::int32_t offset, std::uint32_t extent, std::uint32_t limit)
    {
        return (offset >= 0 && static_cast<std::uint32_t>(offset) + extent > limit);
    };

    if ( IsRegionOutside(textureRegion.offset.x, textureRegion.extent.width,  textureDbg.desc.extent.width ) ||
         IsRegionOutside(textureRegion.offset.y, textureRegion.extent.height, textureDbg.desc.extent.height) ||
         IsRegionOutside(textureRegion.offset.z, textureRegion.extent.depth,  textureDbg.desc.extent.depth ) )
    {
        LLGL_DBG_ERROR(ErrorType::UndefinedBehavior, "texture region exceeded size of texture");
    }
}

enum class DbgTextureFormatCompatibility
{
    Equal,          // Texture formats are equal
    Class1,         // Requires RenderingFeatures::hasTextureViews
    Class2,         // Requires RenderingFeatures::hasTextureViewFormatSwizzle
    Incompatible,   // Incompatible texture formats
};

static DbgTextureFormatCompatibility GetTextureFormatCompatibility(Format sharedTextureFormat, Format textureViewFormat)
{
    if (sharedTextureFormat == textureViewFormat)
        return DbgTextureFormatCompatibility::Equal;

    const FormatAttributes& sharedFormatAttribs = GetFormatAttribs(sharedTextureFormat);
    const FormatAttributes& viewFormatAttribs = GetFormatAttribs(textureViewFormat);

    if (sharedFormatAttribs.bitSize == viewFormatAttribs.bitSize)
    {
        if (sharedFormatAttribs.components == viewFormatAttribs.components)
            return DbgTextureFormatCompatibility::Class1;
        else
            return DbgTextureFormatCompatibility::Class2;
    }

    return DbgTextureFormatCompatibility::Incompatible;
}

void DbgRenderSystem::ValidateTextureView(const DbgTexture& sharedTextureDbg, const TextureViewDescriptor& textureViewDesc)
{
    /* Validate texture-view features are supported */
    if (!GetRenderingCaps().features.hasTextureViews)
        LLGL_DBG_ERROR(ErrorType::UnsupportedFeature, "texture views not supported");
    if (!GetRenderingCaps().features.hasTextureViewSwizzle && !IsTextureSwizzleIdentity(textureViewDesc.swizzle))
        LLGL_DBG_ERROR(ErrorType::UnsupportedFeature, "texture view swizzle not supported, but mapping is not equal to identity");

    /* Validate texture view compatibility */
    const DbgTextureFormatCompatibility formatCompatibility = GetTextureFormatCompatibility(sharedTextureDbg.desc.format, textureViewDesc.format);
    if (formatCompatibility == DbgTextureFormatCompatibility::Incompatible)
    {
        LLGL_DBG_ERROR(
            ErrorType::UnsupportedFeature,
            "texture view format (LLGL::Format::%s) is incompatible with parent format (LLGL::Format::%s)",
            ToString(textureViewDesc.format), ToString(sharedTextureDbg.desc.format)
        );
    }
    else if (formatCompatibility == DbgTextureFormatCompatibility::Class2 && !GetRenderingCaps().features.hasTextureViewFormatSwizzle)
    {
        LLGL_DBG_ERROR(
            ErrorType::UnsupportedFeature,
            "texture view format swizzle not supported, but view format (LLGL::Format::%s) is incompatible with parent format (LLGL::Format::%s)",
            ToString(textureViewDesc.format), ToString(sharedTextureDbg.desc.format)
        );
    }

    /* Validate attributes of shared texture against texture-view descriptor */
    if (sharedTextureDbg.isTextureView)
        LLGL_DBG_ERROR(ErrorType::InvalidArgument, "texture view cannot be shared with another texture view");

    const std::uint32_t mipLevelUpperBound = textureViewDesc.subresource.baseMipLevel + textureViewDesc.subresource.numMipLevels;
    if (mipLevelUpperBound > sharedTextureDbg.mipLevels)
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "texture-view exceeded number of MIP-map levels: %u specified but limit is %u",
            mipLevelUpperBound, sharedTextureDbg.mipLevels
        );
    }

    /* Validate type mapping for texture-view */
    const TextureType srcType = sharedTextureDbg.GetType();
    const TextureType dstType = textureViewDesc.type;

    using T = TextureType;

    switch (srcType)
    {
        case T::Texture1D:
            ValidateTextureViewType(srcType, dstType, { T::Texture1D, T::Texture1DArray });
            break;
        case T::Texture2D:
            ValidateTextureViewType(srcType, dstType, { T::Texture2D, T::Texture2DArray });
            break;
        case T::Texture3D:
            ValidateTextureViewType(srcType, dstType, { T::Texture3D });
            break;
        case T::TextureCube:
            ValidateTextureViewType(srcType, dstType, { T::Texture2D, T::Texture2DArray, T::TextureCube, T::TextureCubeArray });
            break;
        case T::Texture1DArray:
            ValidateTextureViewType(srcType, dstType, { T::Texture1D, T::Texture1DArray });
            break;
        case T::Texture2DArray:
            ValidateTextureViewType(srcType, dstType, { T::Texture2D, T::Texture2DArray });
            break;
        case T::TextureCubeArray:
            ValidateTextureViewType(srcType, dstType, { T::Texture2D, T::Texture2DArray, T::TextureCube, T::TextureCubeArray });
            break;
        case T::Texture2DMS:
            ValidateTextureViewType(srcType, dstType, { T::Texture2DMS, T::Texture2DMSArray });
            break;
        case T::Texture2DMSArray:
            ValidateTextureViewType(srcType, dstType, { T::Texture2DMS, T::Texture2DMSArray });
            break;
    }
}

void DbgRenderSystem::ValidateTextureViewType(const TextureType sharedTextureType, const TextureType textureViewType, const std::initializer_list<TextureType>& validTypes)
{
    if (std::find(validTypes.begin(), validTypes.end(), textureViewType) == validTypes.end())
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "cannot share texture of type <LLGL::TextureType::%s> with texture-view of type <LLGL::TextureType::%s>",
            ToString(sharedTextureType), ToString(textureViewType)
        );
    }
}

void DbgRenderSystem::ValidateAttachmentDesc(const AttachmentDescriptor& attachmentDesc, std::uint32_t colorTarget, bool isResolveAttachment, bool isDepthStencilAttachment)
{
    if (Texture* texture = attachmentDesc.texture)
    {
        auto* textureDbg = LLGL_CAST(DbgTexture*, texture);

        /* Validate attachment type for this texture */
        const Format format = GetAttachmentFormat(attachmentDesc);
        if (IsColorFormat(format))
        {
            if (isDepthStencilAttachment)
            {
                LLGL_DBG_ERROR(
                    ErrorType::InvalidArgument,
                    "cannot use color format for depth-stencil attachment"
                );
            }
            else if ((textureDbg->desc.bindFlags & BindFlags::ColorAttachment) == 0)
            {
                LLGL_DBG_ERROR(
                    ErrorType::InvalidArgument,
                    "cannot have color attachment [%u] with texture that was not created with the 'LLGL::BindFlags::ColorAttachment' flag",
                    colorTarget
                );
            }
        }
        else
        {
            if (isResolveAttachment)
            {
                LLGL_DBG_ERROR(
                    ErrorType::InvalidArgument,
                    "cannot use depth-stencil format for resolve attachment [%u]",
                    colorTarget
                );
            }
            else if (!isDepthStencilAttachment)
            {
                LLGL_DBG_ERROR(
                    ErrorType::InvalidArgument,
                    "cannot use depth-stencil format for color attachment [%u]",
                    colorTarget
                );
            }
            else if ((textureDbg->desc.bindFlags & BindFlags::DepthStencilAttachment) == 0)
            {
                LLGL_DBG_ERROR(
                    ErrorType::InvalidArgument,
                    "cannot have depth-stencil attachment with texture that was not created with the 'LLGL::BindFlags::DepthStencilAttachment' flag"
                );
            }
        }

        /* Validate MIP-level */
        if (attachmentDesc.mipLevel >= textureDbg->mipLevels)
        {
            LLGL_DBG_ERROR(
                ErrorType::InvalidArgument,
                "render-target attachment exceeded number of MIP-map levels: %u specified but upper bound is %u",
                attachmentDesc.mipLevel, textureDbg->mipLevels
            );
        }

        /* Validate array layer */
        if (attachmentDesc.arrayLayer >= textureDbg->desc.arrayLayers)
        {
            LLGL_DBG_ERROR(
                ErrorType::InvalidArgument,
                "render-target attachment exceeded number of array layers: %u specified but upper bound is %u",
                attachmentDesc.arrayLayer, textureDbg->desc.arrayLayers
            );
        }
    }
    else
    {
        if (attachmentDesc.format == Format::Undefined)
        {
            LLGL_DBG_ERROR(
                ErrorType::InvalidArgument,
                "cannot have attachment with undefined format"
            );
        }
    }
}

static std::string GetBindingSlotLabel(const BindingSlot& slot)
{
    std::string label = "slot ";
    label += std::to_string(slot.index);
    if (slot.set > 0)
    {
        label += " (set ";
        label += std::to_string(slot.set);
        label += ')';
    }
    return label;
}

static std::string GetBindingDescLabel(const BindingDescriptor& bindingDesc)
{
    std::string label;
    if (!bindingDesc.name.empty())
    {
        label += '\"';
        label += bindingDesc.name.c_str();
        label += "\" ";
    }
    label += "at ";
    label += GetBindingSlotLabel(bindingDesc.slot);
    return label;
}

static bool IsStreamOutputCompatibleFormat(const Format format)
{
    switch (format)
    {
        case Format::R32Float:
        case Format::RG32Float:
        case Format::RGB32Float:
        case Format::RGBA32Float:
            return true;
        default:
            return false;
    }
}

void DbgRenderSystem::ValidateShaderDesc(const ShaderDescriptor& shaderDesc)
{
    /* Validate shader output-stream attributes */
    std::unordered_map<std::string, std::size_t> attribNameToIndexMap;
    std::unordered_map<SystemValue, std::size_t, EnumHasher<SystemValue>> attribSVToIndexMap;

    struct BufferStrideRef
    {
        std::size_t     firstAttribIndex    = -1;
        std::uint32_t   stride              = 0;
    };
    BufferStrideRef bufferStridesRefs[LLGL_MAX_NUM_SO_BUFFERS];

    const std::string shaderLabelPrefix =
    (
        shaderDesc.debugName != nullptr && *shaderDesc.debugName != '\0'
            ? "shader '" + std::string(shaderDesc.debugName) + "': "
            : ""
    );
    std::string attribLabel;

    for_range(i, shaderDesc.vertex.outputAttribs.size())
    {
        const VertexAttribute& attrib = shaderDesc.vertex.outputAttribs[i];

        /* Construct label for each attribute */;
        attribLabel = shaderLabelPrefix + "stream-output attribute [" + std::to_string(i) + "]";
        if (attrib.systemValue == SystemValue::Undefined && !attrib.name.empty())
            attribLabel += " '" + std::string(attrib.name.c_str()) + "'";

        /* Validate attribute format */
        if (const char* formatIdent = ToString(attrib.format))
        {
            if (!IsStreamOutputCompatibleFormat(attrib.format))
            {
                LLGL_DBG_ERROR(
                    ErrorType::InvalidArgument,
                    "%s format 'LLGL::Format::%s' is not supported in this context",
                    attribLabel.c_str(), formatIdent
                );
            }
        }
        else
        {
            LLGL_DBG_ERROR(
                ErrorType::InvalidArgument,
                "%s format is invalid (%0x08X)",
                attribLabel.c_str(), static_cast<int>(attrib.format)
            );
        }

        /* Validate attribute stride and slot */
        if (attrib.slot < LLGL_MAX_NUM_SO_BUFFERS)
        {
            BufferStrideRef& strideRef = bufferStridesRefs[attrib.slot];
            if (strideRef.firstAttribIndex == -1)
            {
                /* Initialize buffer stride with first attribute that defines it */
                strideRef.firstAttribIndex = i;
                strideRef.stride           = attrib.stride;
            }
            else if (strideRef.stride != attrib.stride)
            {
                LLGL_DBG_ERROR(
                    ErrorType::InvalidArgument,
                    "%s stride mismatch for slot [%u]: %u specified but attribute [%zu] defined it as %u",
                    attribLabel.c_str(), attrib.slot, attrib.stride, strideRef.firstAttribIndex, strideRef.stride
                );
            }
        }
        else
        {
            LLGL_DBG_ERROR(
                ErrorType::InvalidArgument,
                "%s slot index out of bounds: %u specified but upper bound is %u",
                attribLabel.c_str(), attrib.slot, LLGL_MAX_NUM_SO_BUFFERS
            );
        }

        /* Validate attribute name and system-value */
        if (attrib.systemValue != SystemValue::Undefined)
        {
            if (const char* systemValueIdent = ToString(attrib.systemValue))
            {
                auto systemValueIt = attribSVToIndexMap.find(attrib.systemValue);
                if (systemValueIt != attribSVToIndexMap.end())
                {
                    LLGL_DBG_ERROR(
                        ErrorType::InvalidArgument,
                        "%s uses duplicate system-value '%s' that is already defined for attribute [%zu]",
                        attribLabel.c_str(), systemValueIdent, systemValueIt->second
                    );
                }
                else
                    attribSVToIndexMap[attrib.systemValue] = i;
            }
            else
            {
                LLGL_DBG_ERROR(
                    ErrorType::InvalidArgument,
                    "%s uses unknown system-value (0x%08X)",
                    attribLabel.c_str(), static_cast<unsigned>(attrib.systemValue)
                );
            }
        }
        else if (!attrib.name.empty())
        {
            auto nameIt = attribNameToIndexMap.find(attrib.name.c_str());
            if (nameIt != attribNameToIndexMap.end())
            {
                LLGL_DBG_ERROR(
                    ErrorType::InvalidArgument,
                    "%s uses duplicate name '%s' that is already defined for attribute [%zu]",
                    attribLabel.c_str(), attrib.name.c_str(), nameIt->second
                );
            }
            else
                attribNameToIndexMap[attrib.name.c_str()] = i;
        }
        else
        {
            LLGL_DBG_ERROR(
                ErrorType::InvalidArgument,
                "%s defines neither name nor system-value",
                attribLabel.c_str()
            );
        }
    }
}

void DbgRenderSystem::ValidatePipelineLayoutDesc(const PipelineLayoutDescriptor& pipelineLayoutDesc)
{
    /* Validate individual binding descriptors */
    for (const BindingDescriptor& binding : pipelineLayoutDesc.bindings)
    {
        if (binding.arraySize > 1)
        {
            const std::string bindingLabel = GetBindingDescLabel(binding);
            LLGL_DBG_ERROR(
                ErrorType::InvalidArgument,
                "individual binding %s has array size of %u, but only heap-bindings can have an array size other than 0 or 1",
                bindingLabel.c_str(), binding.arraySize
            );
        }
    }
}

void DbgRenderSystem::ValidateResourceHeapDesc(const ResourceHeapDescriptor& resourceHeapDesc, const ArrayView<ResourceViewDescriptor>& initialResourceViews)
{
    if (resourceHeapDesc.pipelineLayout != nullptr)
    {
        auto* pipelineLayoutDbg = LLGL_CAST(DbgPipelineLayout*, resourceHeapDesc.pipelineLayout);
        const auto& bindings = pipelineLayoutDbg->desc.heapBindings;

        const std::uint32_t numResourceViews    = (resourceHeapDesc.numResourceViews > 0 ? resourceHeapDesc.numResourceViews : static_cast<std::uint32_t>(initialResourceViews.size()));
        const std::size_t   numBindings         = bindings.size();

        if (numBindings == 0)
        {
            LLGL_DBG_ERROR(
                ErrorType::InvalidArgument,
                "cannot create resource heap with empty list of heap bindings"
            );
        }
        else if (numResourceViews == 0)
        {
            LLGL_DBG_ERROR(
                ErrorType::InvalidArgument,
                "cannot create resource heap with both 'numResourceViews' being zero and 'initialResourceViews' being empty"
            );
        }
        else if (numResourceViews < numBindings)
        {
            LLGL_DBG_ERROR(
                ErrorType::InvalidArgument,
                "cannot create resource heap with less resources (%u) than bindings in pipeline layout (%zu)",
                numResourceViews, numBindings
            );
        }
        else if (numResourceViews % numBindings != 0)
        {
            LLGL_DBG_ERROR(
                ErrorType::InvalidArgument,
                "cannot create resource heap with number of resource views (%u) not being a multiple of bindings in pipeline layout (%zu)",
                numResourceViews, numBindings
            );
        }
        else if (!initialResourceViews.empty())
        {
            if (initialResourceViews.size() == numResourceViews)
            {
                /* Validate all resource view descriptors against their respective binding descriptor */
                for_range(i, resourceHeapDesc.numResourceViews)
                    ValidateResourceViewForBinding(initialResourceViews[i], bindings[i % bindings.size()]);
            }
            else
            {
                LLGL_DBG_ERROR(
                    ErrorType::InvalidArgument,
                    "mismatch between number of initial resource views and resource heap descriptor: %zu specified but expected %u",
                    initialResourceViews.size(), resourceHeapDesc.numResourceViews
                );
            }
        }
    }
    else
        LLGL_DBG_ERROR(ErrorType::InvalidArgument, "pipeline layout must not be null");
}

void DbgRenderSystem::ValidateResourceHeapRange(const DbgResourceHeap& resourceHeapDbg, std::uint32_t firstDescriptor, const ArrayView<ResourceViewDescriptor>& resourceViews)
{
    if (firstDescriptor >= resourceHeapDbg.desc.numResourceViews)
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "first descriptor in resource heap out of bounds: %u specified but upper bound is %u",
            firstDescriptor, resourceHeapDbg.desc.numResourceViews
        );
    }
    else if (resourceViews.size() + firstDescriptor > resourceHeapDbg.desc.numResourceViews)
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "number of resource views for first descriptor in resource heap out of bounds: %u+%zu specified but limit is %u",
            firstDescriptor, resourceViews.size(), resourceHeapDbg.desc.numResourceViews
        );
    }
}

void DbgRenderSystem::ValidateResourceViewForBinding(const ResourceViewDescriptor& rvDesc, const BindingDescriptor& bindingDesc)
{
    /* Validate stage flags against shader program */
    if (bindingDesc.stageFlags == 0)
        LLGL_DBG_WARN(WarningType::PointlessOperation, "no shader stages are specified for binding descriptor");

    /* Validate resource binding flags */
    if (auto resource = rvDesc.resource)
    {
        switch (resource->GetResourceType())
        {
            case ResourceType::Buffer:
            {
                auto bufferDbg = LLGL_CAST(DbgBuffer*, resource);
                ValidateBufferForBinding(*bufferDbg, bindingDesc);
                if (IsBufferViewEnabled(rvDesc.bufferView))
                    ValidateBufferView(*bufferDbg, rvDesc.bufferView, bindingDesc);
            }
            break;

            case ResourceType::Texture:
            {
                auto textureDbg = LLGL_CAST(DbgTexture*, resource);
                ValidateTextureForBinding(*textureDbg, bindingDesc);
                if (IsTextureViewEnabled(rvDesc.textureView))
                    ValidateTextureView(*textureDbg, rvDesc.textureView);
            }
            break;

            default:
            break;
        }
    }
    else
        LLGL_DBG_ERROR(ErrorType::InvalidArgument, "resource must not be null");
}

void DbgRenderSystem::ValidateBufferForBinding(const DbgBuffer& bufferDbg, const BindingDescriptor& bindingDesc)
{
    if ((bufferDbg.desc.bindFlags & bindingDesc.bindFlags) != bindingDesc.bindFlags)
    {
        const std::string bindingSetLabel = (bindingDesc.slot.set != 0 ? " (set " + std::to_string(bindingDesc.slot.set) + ')' : "");
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "binding flags mismatch between buffer resource at %u%s and binding descriptor",
            bindingDesc.slot.index, bindingSetLabel.c_str()
        );
    }
}

void DbgRenderSystem::ValidateTextureForBinding(const DbgTexture& textureDbg, const BindingDescriptor& bindingDesc)
{
    if ((textureDbg.desc.bindFlags & bindingDesc.bindFlags) != bindingDesc.bindFlags)
    {
        const std::string bindingSetLabel = (bindingDesc.slot.set != 0 ? " (set " + std::to_string(bindingDesc.slot.set) + ')' : "");
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "binding flags mismatch between texture resource at %u%s and binding descriptor",
            bindingDesc.slot.index, bindingSetLabel.c_str()
        );
    }
}

// Converts the specified color mask into a string representation (e.g. "RGBA" or "R_G_").
static std::string ColorMaskToString(std::uint8_t colorMask)
{
    std::string s;
    s.reserve(4);

    s += ((colorMask & ColorMaskFlags::R) != 0 ? 'R' : '_');
    s += ((colorMask & ColorMaskFlags::G) != 0 ? 'G' : '_');
    s += ((colorMask & ColorMaskFlags::B) != 0 ? 'B' : '_');
    s += ((colorMask & ColorMaskFlags::A) != 0 ? 'A' : '_');

    return s;
}

static bool IsBlendOpColorOnly(BlendOp op)
{
    switch (op)
    {
        case BlendOp::SrcColor:
        case BlendOp::InvSrcColor:
        case BlendOp::DstColor:
        case BlendOp::InvDstColor:
        case BlendOp::Src1Color:
        case BlendOp::InvSrc1Color:
            return true;
        default:
            return false;
    }
};

void DbgRenderSystem::ValidateBlendTargetDescriptor(const BlendTargetDescriptor& blendTargetDesc, std::size_t idx)
{
    if (blendTargetDesc.colorMask != 0)
    {
        const std::string colorMaskLabel = ColorMaskToString(blendTargetDesc.colorMask);
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "cannot use color mask {%s} of blend target [%zu] without a fragment shader",
            colorMaskLabel.c_str(), idx
        );
    }
    if (IsBlendOpColorOnly(blendTargetDesc.srcAlpha))
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "cannot use color-only blend operation for source alpha channel (srcAlpha = LLGL::BlendOp::%s)",
            ToString(blendTargetDesc.srcAlpha)
        );
    }
    if (IsBlendOpColorOnly(blendTargetDesc.dstAlpha))
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "cannot use color-only blend operation for destination alpha channel (dstAlpha = LLGL::BlendOp::%s)",
            ToString(blendTargetDesc.dstAlpha)
        );
    }
}

void DbgRenderSystem::ValidateInputAssemblyDescriptor(const GraphicsPipelineDescriptor& pipelineStateDesc)
{
    const Format indexFormat = pipelineStateDesc.indexFormat;
    if (!(indexFormat == Format::Undefined || indexFormat == Format::R16UInt || indexFormat == Format::R32UInt))
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "input assembly index format must be LLGL::Format::Undefined, LLGL::Format::R16UInt, or LLGL::Format::R32UInt, but %s was specified",
            ToString(indexFormat)
        );
    }
}

void DbgRenderSystem::ValidateBlendDescriptor(const BlendDescriptor& blendDesc, bool hasFragmentShader, bool hasDualSourceBlend)
{
    /* Validate proper use of logic pixel operations */
    if (blendDesc.logicOp != LogicOp::Disabled)
    {
        if (!GetRenderingCaps().features.hasLogicOp)
            LLGL_DBG_ERROR(ErrorType::UnsupportedFeature, "logic pixel operations not supported");

        if (blendDesc.independentBlendEnabled)
        {
            LLGL_DBG_ERROR(
                ErrorType::InvalidArgument,
                "logic pixel operations cannot be used in combination with independent blending"
            );
        }

        for (const auto& target : blendDesc.targets)
        {
            if (target.blendEnabled)
            {
                LLGL_DBG_ERROR(
                    ErrorType::InvalidArgument,
                    "logic pixel operations cannot be used in combination with color and alpha blending"
                );
            }
        }
    }

    /* If dual-source blending is enabled, only the first target must be used */
    if (hasDualSourceBlend)
    {
        if (blendDesc.independentBlendEnabled)
        {
            const BlendTargetDescriptor& blendTarget0Desc = blendDesc.targets[0];
            for_subrange(i, 1, LLGL_MAX_NUM_COLOR_ATTACHMENTS)
            {
                if (blendDesc.targets[i].blendEnabled)
                {
                    LLGL_DBG_ERROR(
                        ErrorType::InvalidArgument,
                        "blend target [%u] cannot be enabled in conjunction with dual-source blending,"
                        "but first target is {srcColor=%s, dstColor=%s, srcAlpha=%s, dstAlpha=%s}",
                        i,
                        ToString(blendTarget0Desc.srcColor),
                        ToString(blendTarget0Desc.dstColor),
                        ToString(blendTarget0Desc.srcAlpha),
                        ToString(blendTarget0Desc.dstAlpha)
                    );
                }
            }
        }
    }

    /* Validate color masks are disabled when there is no fragment shader */
    if (!hasFragmentShader)
    {
        if (blendDesc.independentBlendEnabled)
        {
            for_range(i, sizeof(blendDesc.targets) / sizeof(blendDesc.targets[0]))
                ValidateBlendTargetDescriptor(blendDesc.targets[i], i);
        }
        else
            ValidateBlendTargetDescriptor(blendDesc.targets[0], 0);
    }
}

static bool IsDualSourceBlendingOp(BlendOp op)
{
    switch (op)
    {
        case BlendOp::Src1Color:
        case BlendOp::InvSrc1Color:
        case BlendOp::Src1Alpha:
        case BlendOp::InvSrc1Alpha:
            return true;
        default:
            return false;
    }
}

static bool IsDualSourceBlendingEnabled(const BlendTargetDescriptor& blendTarget0Desc)
{
    return
    (
        IsDualSourceBlendingOp(blendTarget0Desc.srcColor) ||
        IsDualSourceBlendingOp(blendTarget0Desc.dstColor) ||
        IsDualSourceBlendingOp(blendTarget0Desc.srcAlpha) ||
        IsDualSourceBlendingOp(blendTarget0Desc.dstAlpha)
    );
}

void DbgRenderSystem::ValidateGraphicsPipelineDesc(const GraphicsPipelineDescriptor& pipelineStateDesc)
{
    const RenderingFeatures& features = GetRenderingCaps().features;
    if (pipelineStateDesc.rasterizer.conservativeRasterization && !features.hasConservativeRasterization)
        LLGL_DBG_ERROR_NOT_SUPPORTED("conservative rasterization");

    /* Validate shader pipeline stages */
    bool hasSeparableShaders = false;
    if (DbgShader* vertexShaderDbg = DbgGetWrapper<DbgShader>(pipelineStateDesc.vertexShader))
        hasSeparableShaders = ((vertexShaderDbg->desc.flags & ShaderCompileFlags::SeparateShader) != 0);
    else
        LLGL_DBG_ERROR(ErrorType::InvalidArgument, "cannot create graphics PSO without vertex shader");

    const bool hasFragmentShader = (pipelineStateDesc.fragmentShader != nullptr);

    if ((pipelineStateDesc.tessControlShader != nullptr) != (pipelineStateDesc.tessEvaluationShader != nullptr))
        LLGL_DBG_ERROR(ErrorType::InvalidArgument, "cannot create graphics PSO with incomplete tessellation shader stages");

    struct ShaderTypePair
    {
        Shader*     shader;
        ShaderType  expectedType;
    };

    SmallVector<DbgShader*, 5> shadersDbg;

    bool hasShadersWithFailedReflection = false;

    for (ShaderTypePair pair : { ShaderTypePair{ pipelineStateDesc.vertexShader,         ShaderType::Vertex         },
                                 ShaderTypePair{ pipelineStateDesc.tessControlShader,    ShaderType::TessControl    },
                                 ShaderTypePair{ pipelineStateDesc.tessEvaluationShader, ShaderType::TessEvaluation },
                                 ShaderTypePair{ pipelineStateDesc.geometryShader,       ShaderType::Geometry       },
                                 ShaderTypePair{ pipelineStateDesc.fragmentShader,       ShaderType::Fragment       } })
    {
        if (Shader* shader = pair.shader)
        {
            auto* shaderDbg = LLGL_CAST(DbgShader*, shader);

            if (shaderDbg->HasReflectionFailed())
                hasShadersWithFailedReflection = true;

            const bool isSeparableShaders = ((shaderDbg->desc.flags & ShaderCompileFlags::SeparateShader) != 0);
            if (isSeparableShaders && !hasSeparableShaders)
            {
                LLGL_DBG_ERROR(
                    ErrorType::InvalidArgument,
                    "cannot mix and match separable %s shader with non-separable shaders in graphics PSO; see LLGL::ShaderCompileFlags::SeparateShader",
                    ToString(shader->GetType())
                );
            }
            else if (!isSeparableShaders && hasSeparableShaders)
            {
                LLGL_DBG_ERROR(
                    ErrorType::InvalidArgument,
                    "cannot mix and match non-separable %s shader with separable shaders in graphics PSO; see LLGL::ShaderCompileFlags::SeparateShader",
                    ToString(shader->GetType())
                );
            }
            if (shader != nullptr && shader->GetType() != pair.expectedType)
            {
                LLGL_DBG_ERROR(
                    ErrorType::InvalidArgument,
                    "cannot create graphics PSO with %s shader being assigned to %s stage",
                    ToString(shader->GetType()), ToString(pair.expectedType)
                );
            }

            shadersDbg.push_back(shaderDbg);
        }
    }

    const bool hasDualSourceBlend = IsDualSourceBlendingEnabled(pipelineStateDesc.blend.targets[0]);

    if (DbgShader* fragmentShaderDbg = DbgGetWrapper<DbgShader>(pipelineStateDesc.fragmentShader))
        ValidateFragmentShaderOutput(*fragmentShaderDbg, pipelineStateDesc.renderPass, hasDualSourceBlend);

    ValidateInputAssemblyDescriptor(pipelineStateDesc);
    ValidateBlendDescriptor(pipelineStateDesc.blend, hasFragmentShader, hasDualSourceBlend);

    if (const DbgPipelineLayout* pipelineLayoutDbg = DbgGetWrapper<DbgPipelineLayout>(pipelineStateDesc.pipelineLayout))
    {
        ValidatePipelineStateUniforms(*pipelineLayoutDbg, shadersDbg, pipelineStateDesc.debugName);

        /* If shader reflection failed, report error if PSO layout requires it (Vulkan specific) */
        if (hasShadersWithFailedReflection && IsVulkan())
        {
            if (!pipelineLayoutDbg->desc.bindings.empty() &&
                !pipelineLayoutDbg->desc.heapBindings.empty())
            {
                const char* psoDebugName = pipelineStateDesc.debugName;
                const std::string psoLabel = (psoDebugName != nullptr && *psoDebugName != '\0' ? " '" + std::string(psoDebugName) + '\'' : "");
                LLGL_DBG_ERROR(
                    ErrorType::UndefinedBehavior,
                    "failed to reflect shader code in PSO%s with mix of heap- and individual bindings; "
                    "perhaps LLGL was built without LLGL_VK_ENABLE_SPIRV_REFLECT",
                    psoLabel.c_str()
                );
            }
        }
    }
}

void DbgRenderSystem::ValidateComputePipelineDesc(const ComputePipelineDescriptor& pipelineStateDesc)
{
    /* Validate shader pipeline stages */
    if (pipelineStateDesc.computeShader != nullptr)
    {
        if (pipelineStateDesc.computeShader->GetType() != ShaderType::Compute)
        {
            LLGL_DBG_ERROR(
                ErrorType::InvalidArgument,
                "cannot create compute PSO with %s shader being assigned to compute stage",
                ToString(pipelineStateDesc.computeShader->GetType())
            );
        }
    }
    else
        LLGL_DBG_ERROR(ErrorType::InvalidArgument, "cannot create compute PSO without compute shader");
}

void DbgRenderSystem::ValidateFragmentShaderOutput(DbgShader& fragmentShaderDbg, const RenderPass* renderPass, bool hasDualSourceBlend)
{
    ShaderReflection reflection;
    if (fragmentShaderDbg.instance.Reflect(reflection))
    {
        if (auto renderPassDbg = DbgGetWrapper<DbgRenderPass>(renderPass))
            ValidateFragmentShaderOutputWithRenderPass(fragmentShaderDbg, reflection.fragment, *renderPassDbg, hasDualSourceBlend);
        else
            ValidateFragmentShaderOutputWithoutRenderPass(fragmentShaderDbg, reflection.fragment);
    }
}

static bool AreFragmentOutputFormatsCompatible(const Format attachmentFormat, const Format attribFormat)
{
    if (attachmentFormat == Format::Undefined || attribFormat == Format::Undefined)
        return false;
    if (IsDepthOrStencilFormat(attachmentFormat) != IsDepthOrStencilFormat(attribFormat))
        return false;
    if (GetFormatAttribs(attachmentFormat).components != GetFormatAttribs(attribFormat).components)
        return false;
    return true;
}

void DbgRenderSystem::ValidateFragmentShaderOutputWithRenderPass(DbgShader& fragmentShaderDbg, const FragmentShaderAttributes& fragmentAttribs, const DbgRenderPass& renderPass, bool hasDualSourceBlend)
{
    const auto numColorAttachments = renderPass.NumEnabledColorAttachments();
    std::uint32_t numColorOutputAttribs = 0u;

    for (const FragmentAttribute& attrib : fragmentAttribs.outputAttribs)
    {
        if (attrib.systemValue == SystemValue::Color)
        {
            if (numColorOutputAttribs >= LLGL_MAX_NUM_COLOR_ATTACHMENTS)
            {
                LLGL_DBG_ERROR(ErrorType::InvalidArgument, "too many color output attributes in fragment shader");
                break;
            }

            const Format attachmentFormat = renderPass.desc.colorAttachments[numColorOutputAttribs].format;

            /* When dual-source blending is enabled, only the first attachment must be specified */
            if (hasDualSourceBlend && numColorOutputAttribs > 0)
            {
                /* Validate this attachment is undefined */
                if (attachmentFormat != Format::Undefined)
                {
                    LLGL_DBG_ERROR(
                        ErrorType::InvalidArgument,
                        "cannot use render pass with color attachment [%u] in conjunction with dual-source blending",
                        numColorOutputAttribs
                    );
                }
            }
            else
            {
                /* Validate this attachment is *NOT* undefined */
                if (attachmentFormat == Format::Undefined)
                {
                    LLGL_DBG_ERROR(
                        ErrorType::InvalidArgument,
                        "cannot use render pass with undefined color attachment [%u] in conjunction with fragment shader that writes to that color target",
                        numColorOutputAttribs
                    );
                }
                else if (!AreFragmentOutputFormatsCompatible(attachmentFormat, attrib.format))
                {
                    LLGL_DBG_ERROR(
                        ErrorType::InvalidArgument,
                        "render pass attachment [%u] format (LLGL::Format::%s) is incompatible with fragment shader output format (LLGL::Format::%s)",
                        numColorOutputAttribs, ToString(attachmentFormat), ToString(attrib.format)
                    );
                }
            }

            ++numColorOutputAttribs;
        }
        else if (attrib.systemValue == SystemValue::Depth        ||
                 attrib.systemValue == SystemValue::DepthGreater ||
                 attrib.systemValue == SystemValue::DepthLess)
        {
            if (renderPass.desc.depthAttachment.format == Format::Undefined)
            {
                LLGL_DBG_ERROR(
                    ErrorType::InvalidArgument,
                    "cannot use render pass with undefined depth attachment in conjunction with fragment shader that writes to the depth buffer"
                );
            }
        }
    }

    if (hasDualSourceBlend)
    {
        if (numColorAttachments != 1)
        {
            LLGL_DBG_ERROR(
                ErrorType::InvalidArgument,
                "render pass for dual-source blending must have exactly 1 color attachment, but %u are specified",
                numColorOutputAttribs
            );
        }
        if (numColorOutputAttribs != 2)
        {
            LLGL_DBG_ERROR(
                ErrorType::InvalidArgument,
                "fragment shader for dual-source blending must have exactly 2 outputs, but %u are specified",
                numColorOutputAttribs
            );
        }
    }
    else
    {
        if (numColorAttachments != numColorOutputAttribs)
        {
            LLGL_DBG_ERROR(
                ErrorType::InvalidArgument,
                "mismatch between number of color attachments in render pass (%u) and fragment shader color outputs (%u)",
                numColorAttachments, numColorOutputAttribs
            );
        }
    }
}

void DbgRenderSystem::ValidateFragmentShaderOutputWithoutRenderPass(DbgShader& fragmentShaderDbg, const FragmentShaderAttributes& fragmentAttribs)
{
    std::uint32_t numColorOutputAttribs = 0u;

    for (const FragmentAttribute& attrib : fragmentAttribs.outputAttribs)
    {
        if (attrib.systemValue == SystemValue::Color)
        {
            if (numColorOutputAttribs >= LLGL_MAX_NUM_COLOR_ATTACHMENTS)
            {
                LLGL_DBG_ERROR(ErrorType::InvalidArgument, "too many color output attributes in fragment shader");
                break;
            }
            ++numColorOutputAttribs;
        }
    }

    if (numColorOutputAttribs > 1)
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "cannot use fragment shader with %u color outputs for PSO without render pass",
            numColorOutputAttribs
        );
    }
}

static ShaderType StageFlagsToShaderType(long stageFlags)
{
    switch (stageFlags)
    {
        case StageFlags::VertexStage:           return ShaderType::Vertex;
        case StageFlags::TessControlStage:      return ShaderType::TessControl;
        case StageFlags::TessEvaluationStage:   return ShaderType::TessEvaluation;
        case StageFlags::GeometryStage:         return ShaderType::Geometry;
        case StageFlags::FragmentStage:         return ShaderType::Fragment;
        case StageFlags::ComputeStage:          return ShaderType::Compute;
        default:                                return ShaderType::Undefined;
    }
}

void DbgRenderSystem::ValidatePipelineStateUniforms(const DbgPipelineLayout& pipelineLayout, const ArrayView<DbgShader*>& shaders, const char* psoDebugName)
{
    /*
    Warn if any uniform is in a cbuffer that has different binding slots across multiple shader stages,
    e.g. implicitly assigned "$Globals" buffer in HLSL with different shader registers across vertex and pixel shader stage.
    */
    if (pipelineLayout.desc.uniforms.empty())
        return;

    std::vector<ShaderReflection> reflections;

    auto FindResourceInPreviousReflectionsByName = [&reflections](const LLGL::StringLiteral& name) -> const ShaderResourceReflection*
    {
        for (const ShaderReflection& otherReflection : reflections)
        {
            for (const ShaderResourceReflection& otherResource : otherReflection.resources)
            {
                if (otherResource.binding.name == name)
                {
                    /* Found matching resource name */
                    return &otherResource;
                }
            }
        }
        return nullptr;
    };

    std::set<std::string> reflectedUniformNames;
    const std::string psoLabel = (psoDebugName != nullptr && *psoDebugName != '\0' ? " '" + std::string(psoDebugName) + '\'' : "");

    for (DbgShader* shader : shaders)
    {
        /* Reflect shader code */
        ShaderReflection reflection;
        if (shader->instance.Reflect(reflection))
        {
            /* Insert reflected uniform names to match against names for all stages */
            for (const UniformDescriptor& unfiromDesc : reflection.uniforms)
                reflectedUniformNames.insert(unfiromDesc.name.c_str());
        }
        else
        {
            LLGL_DBG_ERROR(
                ErrorType::InvalidState,
                "failed to reflect shader code in PSO%s with uniform descriptors%s",
                psoLabel.c_str(),
                (IsVulkan() ? "; perhaps LLGL was built without LLGL_VK_ENABLE_SPIRV_REFLECT" : "")
            );
            continue;
        }

        /* Check mismatch between shader resources */
        if (!reflections.empty())
        {
            for (const ShaderResourceReflection& resource : reflection.resources)
            {
                /* Try to find resource name in other reflections */
                if (const ShaderResourceReflection* otherResource = FindResourceInPreviousReflectionsByName(resource.binding.name))
                {
                    if (otherResource->binding.type != resource.binding.type)
                    {
                        LLGL_DBG_ERROR(
                            ErrorType::InvalidArgument,
                            "type mismatch for resource binding \"%s\" in %s shader (%s) and %s shader (%s)",
                            resource.binding.name.c_str(),
                            ToString(StageFlagsToShaderType(resource.binding.stageFlags)),
                            ToString(resource.binding.type),
                            ToString(StageFlagsToShaderType(otherResource->binding.stageFlags)),
                            ToString(otherResource->binding.type)
                        );
                    }
                    else if (otherResource->binding.slot != resource.binding.slot)
                    {
                        const std::string lhsSlotLabel = GetBindingSlotLabel(resource.binding.slot);
                        const std::string rhsSlotLabel = GetBindingSlotLabel(otherResource->binding.slot);
                        LLGL_DBG_ERROR(
                            ErrorType::InvalidArgument,
                            "slot mismatch for resource binding \"%s\" in %s shader (%s) and %s shader (%s)",
                            resource.binding.name.c_str(),
                            ToString(StageFlagsToShaderType(resource.binding.stageFlags)),
                            lhsSlotLabel.c_str(),
                            ToString(StageFlagsToShaderType(otherResource->binding.stageFlags)),
                            rhsSlotLabel.c_str()
                        );
                    }
                    break;
                }
            }
        }

        /* Append new reflection to list for comparison with other shaders */
        reflections.push_back(std::move(reflection));
    }

    /* Ensure all requested uniforms are included in the code reflection */
    for (const UniformDescriptor& uniformDesc : pipelineLayout.desc.uniforms)
    {
        if (reflectedUniformNames.find(uniformDesc.name.c_str()) == reflectedUniformNames.end())
        {
            LLGL_DBG_ERROR(
                ErrorType::InvalidState,
                "uniform descriptor '%s' was not found in shader code reflection in PSO%s%s",
                uniformDesc.name.c_str(),
                psoLabel.c_str(),
                (IsVulkan() ? "; perhaps LLGL was built without LLGL_VK_ENABLE_SPIRV_REFLECT" : "")
            );
        }
    }
}

void DbgRenderSystem::Assert3DTextures()
{
    const RenderingFeatures& features = GetRenderingCaps().features;
    if (!features.has3DTextures)
        LLGL_DBG_ERROR_NOT_SUPPORTED("3D textures");
}

void DbgRenderSystem::AssertCubeTextures()
{
    const RenderingFeatures& features = GetRenderingCaps().features;
    if (!features.hasCubeTextures)
        LLGL_DBG_ERROR_NOT_SUPPORTED("cube textures");
}

void DbgRenderSystem::AssertArrayTextures()
{
    const RenderingFeatures& features = GetRenderingCaps().features;
    if (!features.hasArrayTextures)
        LLGL_DBG_ERROR_NOT_SUPPORTED("array textures");
}

void DbgRenderSystem::AssertCubeArrayTextures()
{
    const RenderingFeatures& features = GetRenderingCaps().features;
    if (!features.hasCubeArrayTextures)
        LLGL_DBG_ERROR_NOT_SUPPORTED("cube array textures");
}

void DbgRenderSystem::AssertMultiSampleTextures()
{
    const RenderingFeatures& features = GetRenderingCaps().features;
    if (!features.hasMultiSampleTextures)
        LLGL_DBG_ERROR_NOT_SUPPORTED("multi-sample textures");
}

template <typename T, typename TBase>
void DbgRenderSystem::ReleaseDbg(HWObjectContainer<T>& cont, TBase& entry)
{
    auto& entryDbg = LLGL_CAST(T&, entry);
    instance_->Release(entryDbg.instance);
    cont.erase(&entry);
}


} // /namespace LLGL



// ================================================================================
