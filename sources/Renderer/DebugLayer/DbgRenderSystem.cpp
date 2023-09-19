/*
 * DbgRenderSystem.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "DbgRenderSystem.h"
#include "DbgCore.h"
#include "../BufferUtils.h"
#include "../TextureUtils.h"
#include "../CheckedCast.h"
#include "../RenderTargetUtils.h"
#include "../../Core/CoreUtils.h"
#include "../../Core/StringUtils.h"
#include <LLGL/ImageFlags.h>
#include <LLGL/StaticLimits.h>
#include <LLGL/Utils/TypeNames.h>
#include <LLGL/Utils/ForRange.h>


namespace LLGL
{


/*
~~~~~~ INFO ~~~~~~
This is the debug layer render system.
It is a wrapper for the actual render system to validate the parameters, specified by the client programmer.
All the "Create..." and "Write..." functions wrap the function call of the actual render system
into a single braces block to highlight this function call, wher the input parameters are just passed on.
All the actual render system objects are stored in the members named "instance", since they are the actual object instances.
*/

DbgRenderSystem::DbgRenderSystem(RenderSystemPtr&& instance, RenderingProfiler* profiler, RenderingDebugger* debugger) :
    instance_ { std::forward<RenderSystemPtr&&>(instance) },
    profiler_ { profiler                                  },
    debugger_ { debugger                                  },
    caps_     { GetRenderingCaps()                        },
    features_ { caps_.features                            },
    limits_   { caps_.limits                              }
{
    /* Initialize rendering capabilities from wrapped instance */
    UpdateRenderingCaps();
}

/* ----- Swap-chain ----- */

SwapChain* DbgRenderSystem::CreateSwapChain(const SwapChainDescriptor& swapChainDesc, const std::shared_ptr<Surface>& surface)
{
    /* Create primary swap-chain */
    auto swapChainInstance = instance_->CreateSwapChain(swapChainDesc, surface);

    /* Instantiate command queue if not done and update rendering capabilities from wrapped instance */
    if (!commandQueue_)
    {
        UpdateRenderingCaps();
        commandQueue_ = MakeUnique<DbgCommandQueue>(*(instance_->GetCommandQueue()), profiler_, debugger_);
    }

    return swapChains_.emplace<DbgSwapChain>(*swapChainInstance, swapChainDesc);
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
    return commandBuffers_.emplace<DbgCommandBuffer>(
        *instance_,
        commandQueue_->instance,
        *instance_->CreateCommandBuffer(commandBufferDesc),
        debugger_,
        profiler_,
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

    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        ValidateBufferDesc(bufferDesc, &formatSize);
    }

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
        auto bufferDbg          = LLGL_CAST(DbgBuffer*, bufferArray[i]);
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

    if (debugger_)
    {
        LLGL_DBG_SOURCE;

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

    if (profiler_)
        profiler_->frameProfile.bufferWrites++;
}

void DbgRenderSystem::ReadBuffer(Buffer& buffer, std::uint64_t offset, void* data, std::uint64_t dataSize)
{
    auto& bufferDbg = LLGL_CAST(DbgBuffer&, buffer);

    if (debugger_)
    {
        LLGL_DBG_SOURCE;

        if (!bufferDbg.initialized)
            LLGL_DBG_ERROR(ErrorType::InvalidState, "reading uninitialized buffer");
        if (!data)
            LLGL_DBG_ERROR(ErrorType::InvalidArgument, "illegal null pointer argument for 'data' parameter");

        ValidateBufferBoundary(bufferDbg.desc.size, offset, dataSize);
    }

    instance_->ReadBuffer(bufferDbg.instance, offset, data, dataSize);

    if (profiler_)
        profiler_->frameProfile.bufferReads++;
}

void* DbgRenderSystem::MapBuffer(Buffer& buffer, const CPUAccess access)
{
    auto& bufferDbg = LLGL_CAST(DbgBuffer&, buffer);

    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        ValidateResourceCPUAccess(bufferDbg.desc.cpuAccessFlags, access, "buffer");
        ValidateBufferMapping(bufferDbg, true);
    }

    auto result = instance_->MapBuffer(bufferDbg.instance, access);

    if (result != nullptr)
        bufferDbg.OnMap(access, 0, bufferDbg.desc.size);

    if (profiler_)
        profiler_->frameProfile.bufferMappings++;

    return result;
}

void* DbgRenderSystem::MapBuffer(Buffer& buffer, const CPUAccess access, std::uint64_t offset, std::uint64_t length)
{
    auto& bufferDbg = LLGL_CAST(DbgBuffer&, buffer);

    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        ValidateResourceCPUAccess(bufferDbg.desc.cpuAccessFlags, access, "buffer");
        ValidateBufferMapping(bufferDbg, true);
        ValidateBufferBoundary(bufferDbg.desc.size, offset, length);
    }

    auto result = instance_->MapBuffer(bufferDbg.instance, access, offset, length);

    if (result != nullptr)
        bufferDbg.OnMap(access, offset, length);

    if (profiler_)
        profiler_->frameProfile.bufferMappings++;

    return result;
}

void DbgRenderSystem::UnmapBuffer(Buffer& buffer)
{
    auto& bufferDbg = LLGL_CAST(DbgBuffer&, buffer);

    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        ValidateBufferMapping(bufferDbg, false);
    }

    instance_->UnmapBuffer(bufferDbg.instance);

    bufferDbg.OnUnmap();
}

/* ----- Textures ----- */

Texture* DbgRenderSystem::CreateTexture(const TextureDescriptor& textureDesc, const SrcImageDescriptor* imageDesc)
{
    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        ValidateTextureDesc(textureDesc, imageDesc);
    }
    return textures_.emplace<DbgTexture>(*instance_->CreateTexture(textureDesc, imageDesc), textureDesc);
}

void DbgRenderSystem::Release(Texture& texture)
{
    ReleaseDbg(textures_, texture);
}

void DbgRenderSystem::WriteTexture(Texture& texture, const TextureRegion& textureRegion, const SrcImageDescriptor& imageDesc)
{
    auto& textureDbg = LLGL_CAST(DbgTexture&, texture);

    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        ValidateTextureRegion(textureDbg, textureRegion);
        ValidateImageDataSize(textureDbg, textureRegion, imageDesc.format, imageDesc.dataType, imageDesc.dataSize);
    }

    instance_->WriteTexture(textureDbg.instance, textureRegion, imageDesc);

    if (profiler_)
        profiler_->frameProfile.textureWrites++;
}

void DbgRenderSystem::ReadTexture(Texture& texture, const TextureRegion& textureRegion, const DstImageDescriptor& imageDesc)
{
    auto& textureDbg = LLGL_CAST(DbgTexture&, texture);

    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        ValidateTextureRegion(textureDbg, textureRegion);
        ValidateImageDataSize(textureDbg, textureRegion, imageDesc.format, imageDesc.dataType, imageDesc.dataSize);
    }

    instance_->ReadTexture(textureDbg.instance, textureRegion, imageDesc);

    if (profiler_)
        profiler_->frameProfile.textureReads++;
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
        if (auto resource = resourceViewCopy.resource)
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
                    "texture view is enabled in ResourceViewDescriptor[" + std::to_string(i) +  "] but resource is null"
                );
            }
            if (IsBufferViewEnabled(resourceViewCopy.bufferView))
            {
                LLGL_DBG_WARN(
                    WarningType::ImproperArgument,
                    "buffer view is enabled in ResourceViewDescriptor[" + std::to_string(i) +  "] but resource is null"
                );
            }
        }
        instanceResourceViews.push_back(resourceViewCopy);
    }

    return instanceResourceViews;
}

ResourceHeap* DbgRenderSystem::CreateResourceHeap(const ResourceHeapDescriptor& resourceHeapDesc, const ArrayView<ResourceViewDescriptor>& initialResourceViews)
{
    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        ValidateResourceHeapDesc(resourceHeapDesc, initialResourceViews);
    }

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

    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        ValidateResourceHeapRange(resourceHeapDbg, firstDescriptor, resourceViews);
    }

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
    if (auto instance = renderPassDbg.mutableInstance)
    {
        instance_->Release(*instance);
        renderPasses_.erase(&renderPass);
    }
}

/* ----- Render Targets ----- */

RenderTarget* DbgRenderSystem::CreateRenderTarget(const RenderTargetDescriptor& renderTargetDesc)
{
    LLGL_DBG_SOURCE;

    auto instanceDesc = renderTargetDesc;
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
    return renderTargets_.emplace<DbgRenderTarget>(*instance_->CreateRenderTarget(instanceDesc), debugger_, renderTargetDesc);
}

void DbgRenderSystem::Release(RenderTarget& renderTarget)
{
    ReleaseDbg(renderTargets_, renderTarget);
}

/* ----- Shader ----- */

Shader* DbgRenderSystem::CreateShader(const ShaderDescriptor& shaderDesc)
{
    return shaders_.emplace<DbgShader>(*instance_->CreateShader(shaderDesc), shaderDesc);
}

void DbgRenderSystem::Release(Shader& shader)
{
    ReleaseDbg(shaders_, shader);
}

/* ----- Pipeline Layouts ----- */

PipelineLayout* DbgRenderSystem::CreatePipelineLayout(const PipelineLayoutDescriptor& pipelineLayoutDesc)
{
    return pipelineLayouts_.emplace<DbgPipelineLayout>(*instance_->CreatePipelineLayout(pipelineLayoutDesc), pipelineLayoutDesc);
}

void DbgRenderSystem::Release(PipelineLayout& pipelineLayout)
{
    ReleaseDbg(pipelineLayouts_, pipelineLayout);
}

/* ----- Pipeline States ----- */

PipelineState* DbgRenderSystem::CreatePipelineState(const Blob& serializedCache)
{
    return nullptr;//TODO
}

PipelineState* DbgRenderSystem::CreatePipelineState(const GraphicsPipelineDescriptor& pipelineStateDesc, Blob* serializedCache)
{
    LLGL_DBG_SOURCE;

    if (debugger_)
        ValidateGraphicsPipelineDesc(pipelineStateDesc);

    auto instanceDesc = pipelineStateDesc;
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
    return pipelineStates_.emplace<DbgPipelineState>(*instance_->CreatePipelineState(instanceDesc, serializedCache), pipelineStateDesc);
}

PipelineState* DbgRenderSystem::CreatePipelineState(const ComputePipelineDescriptor& pipelineStateDesc, Blob* serializedCache)
{
    LLGL_DBG_SOURCE;

    if (debugger_)
        ValidateComputePipelineDesc(pipelineStateDesc);

    auto instanceDesc = pipelineStateDesc;
    {
        if (pipelineStateDesc.pipelineLayout != nullptr)
            instanceDesc.pipelineLayout = &(LLGL_CAST(const DbgPipelineLayout*, pipelineStateDesc.pipelineLayout)->instance);

        instanceDesc.computeShader = DbgGetInstance<DbgShader>(pipelineStateDesc.computeShader);
    }
    return pipelineStates_.emplace<DbgPipelineState>(*instance_->CreatePipelineState(instanceDesc, serializedCache), pipelineStateDesc);
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
    return instance_->Release(fence);
}

/* ----- Extensions ----- */

bool DbgRenderSystem::GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize)
{
    return instance_->GetNativeHandle(nativeHandle, nativeHandleSize);
}


/*
 * ======= Private: =======
 */

void DbgRenderSystem::ValidateBindFlags(long flags)
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
    if ((flags & bufferOnlyFlags) != 0 && (flags & textureOnlyFlags) != 0)
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
        std::string msg = "unknown CPU access flags specified";
        if (contextDesc)
            msg += (" for " + std::string(contextDesc));
        LLGL_DBG_WARN(WarningType::ImproperArgument, msg);
    }
}

void DbgRenderSystem::ValidateMiscFlags(long flags, long validFlags, const char* contextDesc)
{
    if ((flags & (~validFlags)) != 0)
    {
        std::string msg = "unknown miscellaneous flags specified";
        if (contextDesc)
            msg += (" for " + std::string(contextDesc));
        LLGL_DBG_WARN(WarningType::ImproperArgument, msg);
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
                "cannot map " + std::string(resourceTypeName) + " with CPU read access, because the resource was not created with 'LLGL::CPUAccessFlags::Read' flag"
            );
        }
    }
    if (access == CPUAccess::WriteOnly || access == CPUAccess::ReadWrite)
    {
        if ((cpuAccessFlags & CPUAccessFlags::Write) == 0)
        {
            LLGL_DBG_ERROR(
                ErrorType::InvalidState,
                "cannot map " + std::string(resourceTypeName) + " with CPU write access, because the resource was not created with 'LLGL::CPUAccessFlags::Write' flag"
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

    /* Validate number of native buffers */
    if (commandBufferDesc.numNativeBuffers == 0)
        LLGL_DBG_ERROR(ErrorType::InvalidArgument, "cannot create command buffer with zero native buffers");
}

void DbgRenderSystem::ValidateBufferDesc(const BufferDescriptor& bufferDesc, std::uint32_t* formatSizeOut)
{
    /* Validate flags */
    ValidateBindFlags(bufferDesc.bindFlags);
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

        /* Validate buffer size for specified vertex format */
        formatSize = bufferDesc.vertexAttribs.front().stride;
        if (formatSize > 0 && bufferDesc.size % formatSize != 0)
            LLGL_DBG_WARN(WarningType::ImproperArgument, "improper vertex buffer size with vertex format of " + std::to_string(formatSize) + " bytes");
    }

    if ((bufferDesc.bindFlags & BindFlags::IndexBuffer) != 0 && bufferDesc.format != Format::Undefined)
    {
        /* Validate index format */
        if (bufferDesc.format != Format::R16UInt &&
            bufferDesc.format != Format::R32UInt)
        {
            if (auto formatName = ToString(bufferDesc.format))
                LLGL_DBG_ERROR(ErrorType::InvalidArgument, "invalid index buffer format: LLGL::Format::" + std::string(formatName));
            else
                LLGL_DBG_ERROR(ErrorType::InvalidArgument, "unknown index buffer format: " + std::string(IntToHex(static_cast<std::uint32_t>(bufferDesc.format))));
        }

        /* Validate buffer size for specified index format */
        formatSize = GetFormatAttribs(bufferDesc.format).bitSize / 8;
        if (formatSize > 0 && bufferDesc.size % formatSize != 0)
        {
            LLGL_DBG_WARN(
                WarningType::ImproperArgument,
                "improper index buffer size with index format of " + std::to_string(formatSize) + " bytes"
            );
        }
    }

    if ((bufferDesc.bindFlags & BindFlags::ConstantBuffer) != 0)
    {
        /* Validate pack alginemnt of 16 bytes */
        static const std::uint64_t packAlignment = 16;
        if (bufferDesc.size % packAlignment != 0)
            LLGL_DBG_WARN(WarningType::ImproperArgument, "constant buffer size is out of pack alignment (alignment is 16 bytes)");
    }

    /* Validate buffer stride */
    if (bufferDesc.stride > 0 && bufferDesc.size % bufferDesc.stride != 0)
        LLGL_DBG_ERROR(ErrorType::InvalidArgument, "buffer stride is greater than zero, but size is not a multiple of stride");

    if (formatSizeOut)
        *formatSizeOut = formatSize;
}

void DbgRenderSystem::ValidateVertexAttributesForBuffer(const VertexAttribute& lhs, const VertexAttribute& rhs)
{
    if (lhs.slot != rhs.slot || lhs.stride != rhs.stride || lhs.instanceDivisor != rhs.instanceDivisor)
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "vertex attributes must have equal slot, stride, and instance divisor within the same buffer, "
            "but found mismatch between \"" + lhs.name + "\" and \"" + rhs.name + "\""
        );
    }
}

void DbgRenderSystem::ValidateBufferSize(std::uint64_t size)
{
    if (size > limits_.maxBufferSize)
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "buffer size exceeded limit (" + std::to_string(size) +
            " specified but limit is " + std::to_string(limits_.maxBufferSize) + ")"
        );
    }
}

void DbgRenderSystem::ValidateConstantBufferSize(std::uint64_t size)
{
    if (size > limits_.maxConstantBufferSize)
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "constant buffer size exceeded limit (" + std::to_string(size) +
            " specified but limit is " + std::to_string(limits_.maxConstantBufferSize) + ")"
        );
    }
}

void DbgRenderSystem::ValidateBufferBoundary(std::uint64_t bufferSize, std::uint64_t dstOffset, std::uint64_t dataSize)
{
    if (dstOffset >= bufferSize)
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "buffer offset out of bounds (" + std::to_string(dstOffset) +
            " specified but upper bound is " + std::to_string(bufferSize) + ")"
        );
    }
    else if (dataSize + dstOffset > bufferSize)
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "data size for buffer offset out of bounds (" + std::to_string(dstOffset) + "+" + std::to_string(dataSize) +
            " specified but limit is " + std::to_string(bufferSize) + ")"
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

static std::string BindingSlotToString(const BindingSlot& slot)
{
    std::string s;
    s += "slot ";
    s += std::to_string(slot.index);
    if (slot.set != 0)
    {
        s += " (set ";
        s += std::to_string(slot.set);
        s += ')';
    }
    return s;
}

void DbgRenderSystem::ValidateBufferView(DbgBuffer& bufferDbg, const BufferViewDescriptor& viewDesc, const BindingDescriptor& bindingDesc)
{
    const std::uint64_t minAlignment = GetMinAlignmentForBufferBinding(bindingDesc, limits_);
    if (minAlignment > 0 && (viewDesc.offset % minAlignment != 0 || viewDesc.size % minAlignment != 0))
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "buffer view '" + bindingDesc.name + "' at " + BindingSlotToString(bindingDesc.slot) +
            " does not satisfy minimum alignment of " + std::to_string(minAlignment) + " bytes"
        );
    }
}

void DbgRenderSystem::ValidateTextureDesc(const TextureDescriptor& textureDesc, const SrcImageDescriptor* imageDesc)
{
    switch (textureDesc.type)
    {
        case TextureType::Texture1D:
            Validate1DTextureSize(textureDesc.extent.width);
            ValidateTextureSizeDefault(textureDesc.extent.height);
            ValidateTextureSizeDefault(textureDesc.extent.depth);
            break;

        case TextureType::Texture2D:
            Validate2DTextureSize(textureDesc.extent.width);
            Validate2DTextureSize(textureDesc.extent.height);
            ValidateTextureSizeDefault(textureDesc.extent.depth);
            break;

        case TextureType::TextureCube:
            AssertCubeTextures();
            ValidateCubeTextureSize(textureDesc.extent.width, textureDesc.extent.height);
            ValidateTextureSizeDefault(textureDesc.extent.depth);
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
            ValidateTextureSizeDefault(textureDesc.extent.height);
            ValidateTextureSizeDefault(textureDesc.extent.depth);
            break;

        case TextureType::Texture2DArray:
            AssertArrayTextures();
            Validate1DTextureSize(textureDesc.extent.width);
            Validate1DTextureSize(textureDesc.extent.height);
            ValidateTextureSizeDefault(textureDesc.extent.depth);
            break;

        case TextureType::TextureCubeArray:
            AssertCubeArrayTextures();
            ValidateCubeTextureSize(textureDesc.extent.width, textureDesc.extent.height);
            ValidateTextureSizeDefault(textureDesc.extent.depth);
            break;

        case TextureType::Texture2DMS:
            AssertMultiSampleTextures();
            Validate2DTextureSize(textureDesc.extent.width);
            Validate2DTextureSize(textureDesc.extent.height);
            ValidateTextureSizeDefault(textureDesc.extent.depth);
            break;

        case TextureType::Texture2DMSArray:
            AssertMultiSampleTextures();
            AssertArrayTextures();
            Validate2DTextureSize(textureDesc.extent.width);
            Validate2DTextureSize(textureDesc.extent.height);
            ValidateTextureSizeDefault(textureDesc.extent.depth);
            break;

        default:
            LLGL_DBG_ERROR(ErrorType::InvalidArgument, "invalid texture type");
            break;
    }

    ValidateTextureFormatSupported(textureDesc.format);
    ValidateTextureDescMipLevels(textureDesc);
    ValidateArrayTextureLayers(textureDesc.type, textureDesc.arrayLayers);
    ValidateBindFlags(textureDesc.bindFlags);
    ValidateMiscFlags(textureDesc.miscFlags, (MiscFlags::DynamicUsage | MiscFlags::FixedSamples | MiscFlags::GenerateMips | MiscFlags::NoInitialData), "texture");

    /* Check if MIP-map generation is requested  */
    if ((textureDesc.miscFlags & MiscFlags::GenerateMips) != 0)
    {
        if (imageDesc == nullptr)
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
            "cannot create texture with unsupported format: " + std::string(ToString(format))
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
                "number of MIP-map levels exceeded limit (" + std::to_string(textureDesc.mipLevels) +
                " specified but limit is " + std::to_string(maxNumMipLevels) + ")"
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
            std::string(textureTypeName) + " texture size exceeded limit (" + std::to_string(size) +
            " specified but limit is " + std::to_string(limit) + ")"
        );
    }
}

void DbgRenderSystem::ValidateTextureSizeDefault(std::uint32_t size)
{
    if (size == 0)
        LLGL_DBG_ERROR(ErrorType::InvalidArgument, "texture size must not be 0");
    if (size > 1)
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "unused texture dimension must be one (but " + std::to_string(size) + " was specified)"
        );
    }
}

void DbgRenderSystem::Validate1DTextureSize(std::uint32_t size)
{
    ValidateTextureSize(size, limits_.max1DTextureSize, "1D");
}

void DbgRenderSystem::Validate2DTextureSize(std::uint32_t size)
{
    ValidateTextureSize(size, limits_.max2DTextureSize, "2D");
}

void DbgRenderSystem::Validate3DTextureSize(std::uint32_t size)
{
    ValidateTextureSize(size, limits_.max3DTextureSize, "3D");
}

void DbgRenderSystem::ValidateCubeTextureSize(std::uint32_t width, std::uint32_t height)
{
    ValidateTextureSize(width, limits_.maxCubeTextureSize, "cube");
    ValidateTextureSize(height, limits_.maxCubeTextureSize, "cube");
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
                        "number of texture layers must be 6 for cube textures (but " +
                        std::to_string(layers) + " was specified)"
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
                        "number of texture layers must be a multiple of 6 for cube array textures (but " +
                        std::to_string(layers) + " was specified)"
                    );
                }
            }
            break;

            default:
            {
                if (IsArrayTexture(type))
                {
                    const auto maxNumLayers = limits_.maxTextureArrayLayers;
                    if (layers > maxNumLayers)
                    {
                        LLGL_DBG_ERROR(
                            ErrorType::InvalidArgument,
                            "number of texture layers exceeded limit (" + std::to_string(layers) +
                            " specified but limit is " + std::to_string(maxNumLayers) + ")"
                        );
                    }
                }
                else
                {
                    LLGL_DBG_ERROR(
                        ErrorType::InvalidArgument,
                        "number of texture array layers must be 1 for non-array textures (but " +
                        std::to_string(layers) + " was specified)"
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
            "mip level out of bounds (" + std::to_string(baseMipLevel + numMipLevels) +
            " exceeded limit of " + std::to_string(numMipLevels) + ")"
        );
    }
}

//TODO: also support compressed formats in validation
void DbgRenderSystem::ValidateImageDataSize(const DbgTexture& textureDbg, const TextureRegion& textureRegion, ImageFormat imageFormat, DataType dataType, std::size_t dataSize)
{
    /* Validate output data size */
    const auto&         subresource         = textureRegion.subresource;
    const auto          baseSubresource     = TextureSubresource{ 0, subresource.numArrayLayers, 0, subresource.numMipLevels };
    const auto          numTexels           = NumMipTexels(textureDbg.desc.type, textureRegion.extent, baseSubresource);
    const std::size_t   requiredDataSize    = GetMemoryFootprint(imageFormat, dataType, numTexels);

    /* Ignore compressed formats */
    if (requiredDataSize != 0)
    {
        if (dataSize < requiredDataSize)
        {
            LLGL_DBG_ERROR(
                ErrorType::InvalidArgument,
                "image data size too small for texture: " + std::to_string(dataSize) +
                " byte(s) specified but required is " + std::to_string(requiredDataSize) + " byte(s)"
            );
        }
        else if (dataSize > requiredDataSize)
        {
            LLGL_DBG_WARN(
                WarningType::ImproperArgument,
                "image data size larger than expected for texture: " + std::to_string(dataSize) +
                " byte(s) specified but required is " + std::to_string(requiredDataSize) + " byte(s)"
            );
        }
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
    const auto arrayLayerRangeEnd = baseArrayLayer + numArrayLayers;
    if (arrayLayerRangeEnd > arrayLayerLimit)
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "array layer out of range for array texture (" + std::to_string(arrayLayerRangeEnd) +
            " specified but limit is " + std::to_string(arrayLayerLimit) + ")"
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
    {
        LLGL_DBG_ERROR(
            ErrorType::UndefinedBehavior,
            "negative offset not allowed to write a texture region"
        );
    }

    /* Validate offset plus extent */
    auto IsRegionOutside = [](std::int32_t offset, std::uint32_t extent, std::uint32_t limit)
    {
        return (offset >= 0 && static_cast<std::uint32_t>(offset) + extent > limit);
    };

    if ( IsRegionOutside(textureRegion.offset.x, textureRegion.extent.width,  textureDbg.desc.extent.width ) ||
         IsRegionOutside(textureRegion.offset.y, textureRegion.extent.height, textureDbg.desc.extent.height) ||
         IsRegionOutside(textureRegion.offset.z, textureRegion.extent.depth,  textureDbg.desc.extent.depth ) )
    {
        LLGL_DBG_ERROR(
            ErrorType::UndefinedBehavior,
            "texture region exceeded size of texture"
        );
    }
}

void DbgRenderSystem::ValidateTextureView(const DbgTexture& sharedTextureDbg, const TextureViewDescriptor& textureViewDesc)
{
    /* Validate texture-view features are supported */
    if (!GetRenderingCaps().features.hasTextureViews)
        LLGL_DBG_ERROR(ErrorType::UnsupportedFeature, "texture views not supported");
    if (!GetRenderingCaps().features.hasTextureViewSwizzle && !IsTextureSwizzleIdentity(textureViewDesc.swizzle))
        LLGL_DBG_ERROR(ErrorType::UnsupportedFeature, "texture view swizzle not supported, but mapping is not equal to identity");

    /* Validate attributes of shared texture against texture-view descriptor */
    if (sharedTextureDbg.isTextureView)
        LLGL_DBG_ERROR(ErrorType::InvalidArgument, "texture view cannot be shared with another texture view");

    const auto mipLevelUpperBound = textureViewDesc.subresource.baseMipLevel + textureViewDesc.subresource.numMipLevels;
    if (mipLevelUpperBound > sharedTextureDbg.mipLevels)
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "texture-view exceeded number of MIP-map levels (" +
            std::to_string(mipLevelUpperBound) + " specified but limit is " + std::to_string(sharedTextureDbg.mipLevels) + ")"
        );
    }

    /* Validate type mapping for texture-view */
    const auto srcType = sharedTextureDbg.GetType();
    const auto dstType = textureViewDesc.type;

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
            "cannot share texture of type <" + std::string(ToString(sharedTextureType)) +
            "> with texture-view of type <" + std::string(ToString(textureViewType)) + ">"
        );
    }
}

void DbgRenderSystem::ValidateAttachmentDesc(const AttachmentDescriptor& attachmentDesc, std::uint32_t colorTarget, bool isResolveAttachment, bool isDepthStencilAttachment)
{
    if (auto texture = attachmentDesc.texture)
    {
        auto textureDbg = LLGL_CAST(DbgTexture*, texture);

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
                    "cannot have color attachment [" + std::to_string(colorTarget) +
                    "] with texture that was not created with the 'LLGL::BindFlags::ColorAttachment' flag"
                );
            }
        }
        else
        {
            if (isResolveAttachment)
            {
                LLGL_DBG_ERROR(
                    ErrorType::InvalidArgument,
                    "cannot use depth-stencil format for resolve attachment [" + std::to_string(colorTarget) + "]"
                );
            }
            else if (!isDepthStencilAttachment)
            {
                LLGL_DBG_ERROR(
                    ErrorType::InvalidArgument,
                    "cannot use depth-stencil format for color attachment [" + std::to_string(colorTarget) + "]"
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
                "render-target attachment exceeded number of MIP-map levels (" +
                std::to_string(attachmentDesc.mipLevel) + " specified but upper bound is " + std::to_string(textureDbg->mipLevels) + ")"
            );
        }

        /* Validate array layer */
        if (attachmentDesc.arrayLayer >= textureDbg->desc.arrayLayers)
        {
            LLGL_DBG_ERROR(
                ErrorType::InvalidArgument,
                "render-target attachment exceeded number of array layers (" +
                std::to_string(attachmentDesc.arrayLayer) + " specified but upper bound is " + std::to_string(textureDbg->desc.arrayLayers) + ")"
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

void DbgRenderSystem::ValidateResourceHeapDesc(const ResourceHeapDescriptor& resourceHeapDesc, const ArrayView<ResourceViewDescriptor>& initialResourceViews)
{
    if (resourceHeapDesc.pipelineLayout != nullptr)
    {
        auto pipelineLayoutDbg = LLGL_CAST(DbgPipelineLayout*, resourceHeapDesc.pipelineLayout);
        const auto& bindings = pipelineLayoutDbg->desc.heapBindings;

        const auto numResourceViews = (resourceHeapDesc.numResourceViews > 0 ? resourceHeapDesc.numResourceViews : static_cast<std::uint32_t>(initialResourceViews.size()));
        const auto numBindings      = bindings.size();

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
                "cannot create resource heap with less resources (" + std::to_string(numResourceViews) +
                ") than bindings in pipeline layout (" + std::to_string(numBindings) + ")"
            );
        }
        else if (numResourceViews % numBindings != 0)
        {
            LLGL_DBG_ERROR(
                ErrorType::InvalidArgument,
                "cannot create resource heap with number of resource views (" + std::to_string(numResourceViews) +
                ") not being a multiple of bindings in pipeline layout (" + std::to_string(numBindings) + ")"
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
                    "mismatch between number of initial resource views and resource heap descriptor (" +
                    std::to_string(initialResourceViews.size()) + " specified but expected " +
                    std::to_string(resourceHeapDesc.numResourceViews) + ")"
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
            "first descriptor in resource heap out of bounds (" + std::to_string(firstDescriptor) +
            " specified but upper bound is " + std::to_string(resourceHeapDbg.desc.numResourceViews) + ")"
        );
    }
    else if (resourceViews.size() + firstDescriptor > resourceHeapDbg.desc.numResourceViews)
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "number of resource views for first descriptor in resource heap out of bounds (" +
            std::to_string(firstDescriptor) + "+" + std::to_string(resourceViews.size()) +
            " specified but limit is " + std::to_string(resourceHeapDbg.desc.numResourceViews) + ")"
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
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "binding flags mismatch between buffer resource at " +
            BindingSlotToString(bindingDesc.slot) + " and binding descriptor"
        );
    }
}

void DbgRenderSystem::ValidateTextureForBinding(const DbgTexture& textureDbg, const BindingDescriptor& bindingDesc)
{
    if ((textureDbg.desc.bindFlags & bindingDesc.bindFlags) != bindingDesc.bindFlags)
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "binding flags mismatch between texture resource at " +
            BindingSlotToString(bindingDesc.slot) + " and binding descriptor"
        );
    }
}

// Converts the specified color mask into a string representation (e.g. "RGBA" or "R_G_").
static std::string ColorMaskToString(std::uint8_t colorMask)
{
    std::string s;

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
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "cannot use color mask <" + ColorMaskToString(blendTargetDesc.colorMask) +
            "> of blend target <" + std::to_string(idx) + "> without a fragment shader"
        );
    }
    if (IsBlendOpColorOnly(blendTargetDesc.srcAlpha))
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "cannot use color-only blend operation for source alpha channel (srcAlpha = LLGL::BlendOp::" + std::string(ToString(blendTargetDesc.srcAlpha)) + ")"
        );
    }
    if (IsBlendOpColorOnly(blendTargetDesc.dstAlpha))
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "cannot use color-only blend operation for destination alpha channel (dstAlpha = LLGL::BlendOp::" + std::string(ToString(blendTargetDesc.dstAlpha)) + ")"
        );
    }
}

void DbgRenderSystem::ValidateBlendDescriptor(const BlendDescriptor& blendDesc, bool hasFragmentShader)
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

void DbgRenderSystem::ValidateGraphicsPipelineDesc(const GraphicsPipelineDescriptor& pipelineStateDesc)
{
    if (pipelineStateDesc.rasterizer.conservativeRasterization && !features_.hasConservativeRasterization)
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

    for (ShaderTypePair pair : { ShaderTypePair{ pipelineStateDesc.vertexShader,         ShaderType::Vertex         },
                                 ShaderTypePair{ pipelineStateDesc.tessControlShader,    ShaderType::TessControl    },
                                 ShaderTypePair{ pipelineStateDesc.tessEvaluationShader, ShaderType::TessEvaluation },
                                 ShaderTypePair{ pipelineStateDesc.geometryShader,       ShaderType::Geometry       },
                                 ShaderTypePair{ pipelineStateDesc.fragmentShader,       ShaderType::Fragment       } })
    {
        if (Shader* shader = pair.shader)
        {
            auto shaderDbg = LLGL_CAST(DbgShader*, shader);
            const bool isSeparableShaders = ((shaderDbg->desc.flags & ShaderCompileFlags::SeparateShader) != 0);
            if (isSeparableShaders && !hasSeparableShaders)
            {
                LLGL_DBG_ERROR(
                    ErrorType::InvalidArgument,
                    "cannot mix and match separable " + std::string(ToString(shader->GetType())) +
                    " shader with non-separable shaders in graphics PSO; see LLGL::ShaderCompileFlags::SeparateShader"
                );
            }
            else if (!isSeparableShaders && hasSeparableShaders)
            {
                LLGL_DBG_ERROR(
                    ErrorType::InvalidArgument,
                    "cannot mix and match non-separable " + std::string(ToString(shader->GetType())) +
                    " shader with separable shaders in graphics PSO; see LLGL::ShaderCompileFlags::SeparateShader"
                );
            }
            if (shader != nullptr && shader->GetType() != pair.expectedType)
            {
                LLGL_DBG_ERROR(
                    ErrorType::InvalidArgument,
                    "cannot create graphics PSO with " + std::string(ToString(shader->GetType())) +
                    " shader being assigned to " + std::string(ToString(pair.expectedType)) + " stage"
                );
            }
        }
    }

    if (DbgShader* fragmentShaderDbg = DbgGetWrapper<DbgShader>(pipelineStateDesc.fragmentShader))
        ValidateFragmentShaderOutput(*fragmentShaderDbg, pipelineStateDesc.renderPass);

    ValidateBlendDescriptor(pipelineStateDesc.blend, hasFragmentShader);
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
                "cannot create compute PSO with " + std::string(ToString(pipelineStateDesc.computeShader->GetType())) +
                " shader being assigned to compute stage"
            );
        }
    }
    else
        LLGL_DBG_ERROR(ErrorType::InvalidArgument, "cannot create compute PSO without compute shader");
}

void DbgRenderSystem::ValidateFragmentShaderOutput(DbgShader& fragmentShaderDbg, const RenderPass* renderPass)
{
    ShaderReflection reflection;
    if (fragmentShaderDbg.instance.Reflect(reflection))
    {
        if (auto renderPassDbg = DbgGetWrapper<DbgRenderPass>(renderPass))
            ValidateFragmentShaderOutputWithRenderPass(fragmentShaderDbg, reflection.fragment, *renderPassDbg);
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

void DbgRenderSystem::ValidateFragmentShaderOutputWithRenderPass(DbgShader& fragmentShaderDbg, const FragmentShaderAttributes& fragmentAttribs, const DbgRenderPass& renderPass)
{
    const auto numColorAttachments = renderPass.NumEnabledColorAttachments();
    std::uint32_t numColorOutputAttribs = 0u;

    for (const auto& attrib : fragmentAttribs.outputAttribs)
    {
        if (attrib.systemValue == SystemValue::Color)
        {
            if (numColorOutputAttribs >= LLGL_MAX_NUM_COLOR_ATTACHMENTS)
            {
                LLGL_DBG_ERROR(ErrorType::InvalidArgument, "too many color output attributes in fragment shader");
                break;
            }
            const Format attachmentFormat = renderPass.desc.colorAttachments[numColorOutputAttribs].format;
            if (attachmentFormat == Format::Undefined)
            {
                LLGL_DBG_ERROR(
                    ErrorType::InvalidArgument,
                    "cannot use render pass with undefined color attachment [" + std::to_string(numColorOutputAttribs) +
                    "] in conjunction with fragment shader that writes to that color target"
                );
            }
            else if (!AreFragmentOutputFormatsCompatible(attachmentFormat, attrib.format))
            {
                LLGL_DBG_ERROR(
                    ErrorType::InvalidArgument,
                    "render pass attachment [" + std::to_string(numColorOutputAttribs) + "] format (" + std::string(ToString(attachmentFormat)) +
                    ") is incompatible with fragment shader output format (" + std::string(ToString(attrib.format)) + ")"
                );
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

    if (numColorAttachments != numColorOutputAttribs)
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "mismatch between number of color attachments in render pass (" + std::to_string(numColorAttachments) +
            ") and fragment shader color outputs (" + std::to_string(numColorOutputAttribs) + ")"
        );
    }
}

void DbgRenderSystem::ValidateFragmentShaderOutputWithoutRenderPass(DbgShader& fragmentShaderDbg, const FragmentShaderAttributes& fragmentAttribs)
{
    std::uint32_t numColorOutputAttribs = 0u;

    for (const auto& attrib : fragmentAttribs.outputAttribs)
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
            "cannot use fragment shader with " + std::to_string(numColorOutputAttribs) + " color outputs for PSO without render pass"
        );
    }
}

void DbgRenderSystem::Assert3DTextures()
{
    if (!features_.has3DTextures)
        LLGL_DBG_ERROR_NOT_SUPPORTED("3D textures");
}

void DbgRenderSystem::AssertCubeTextures()
{
    if (!features_.hasCubeTextures)
        LLGL_DBG_ERROR_NOT_SUPPORTED("cube textures");
}

void DbgRenderSystem::AssertArrayTextures()
{
    if (!features_.hasArrayTextures)
        LLGL_DBG_ERROR_NOT_SUPPORTED("array textures");
}

void DbgRenderSystem::AssertCubeArrayTextures()
{
    if (!features_.hasCubeArrayTextures)
        LLGL_DBG_ERROR_NOT_SUPPORTED("cube array textures");
}

void DbgRenderSystem::AssertMultiSampleTextures()
{
    if (!features_.hasMultiSampleTextures)
        LLGL_DBG_ERROR_NOT_SUPPORTED("multi-sample textures");
}

template <typename T, typename TBase>
void DbgRenderSystem::ReleaseDbg(HWObjectContainer<T>& cont, TBase& entry)
{
    auto& entryDbg = LLGL_CAST(T&, entry);
    instance_->Release(entryDbg.instance);
    cont.erase(&entry);
}

void DbgRenderSystem::UpdateRenderingCaps()
{
    /* Store meta data about render system */
    SetRendererInfo(instance_->GetRendererInfo());
    SetRenderingCaps(instance_->GetRenderingCaps());
}


} // /namespace LLGL



// ================================================================================
