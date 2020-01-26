/*
 * DbgRenderSystem.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "DbgRenderSystem.h"
#include "DbgCore.h"
#include "../BufferUtils.h"
#include "../TextureUtils.h"
#include "../CheckedCast.h"
#include "../../Core/Helper.h"
#include <LLGL/Strings.h>
#include <LLGL/ImageFlags.h>
#include <LLGL/StaticLimits.h>


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

DbgRenderSystem::DbgRenderSystem(
    const std::shared_ptr<RenderSystem>&    instance,
    RenderingProfiler*                      profiler,
    RenderingDebugger*                      debugger)
:
    instance_ { instance           },
    profiler_ { profiler           },
    debugger_ { debugger           },
    caps_     { GetRenderingCaps() },
    features_ { caps_.features     },
    limits_   { caps_.limits       }
{
}

void DbgRenderSystem::SetConfiguration(const RenderSystemConfiguration& config)
{
    RenderSystem::SetConfiguration(config);
    instance_->SetConfiguration(config);
}

/* ----- Render Context ----- */

RenderContext* DbgRenderSystem::CreateRenderContext(const RenderContextDescriptor& desc, const std::shared_ptr<Surface>& surface)
{
    /* Create primary render context */
    auto renderContextInstance = instance_->CreateRenderContext(desc, surface);

    if (!commandQueue_)
    {
        /* Store meta data about render system */
        SetRendererInfo(instance_->GetRendererInfo());
        SetRenderingCaps(instance_->GetRenderingCaps());

        /* Instantiate command queue */
        commandQueue_ = MakeUnique<DbgCommandQueue>(*(instance_->GetCommandQueue()), profiler_, debugger_);
    }

    return TakeOwnership(renderContexts_, MakeUnique<DbgRenderContext>(*renderContextInstance));
}

void DbgRenderSystem::Release(RenderContext& renderContext)
{
    ReleaseDbg(renderContexts_, renderContext);
}

/* ----- Command queues ----- */

CommandQueue* DbgRenderSystem::GetCommandQueue()
{
    return commandQueue_.get();
}

/* ----- Command buffers ----- */

CommandBuffer* DbgRenderSystem::CreateCommandBuffer(const CommandBufferDescriptor& desc)
{
    return TakeOwnership(
        commandBuffers_,
        MakeUnique<DbgCommandBuffer>(
            *instance_,
            commandQueue_->instance,
            *instance_->CreateCommandBuffer(desc),
            debugger_,
            profiler_,
            desc,
            GetRenderingCaps()
        )
    );
}

void DbgRenderSystem::Release(CommandBuffer& commandBuffer)
{
    ReleaseDbg(commandBuffers_, commandBuffer);
}

/* ----- Buffers ------ */

Buffer* DbgRenderSystem::CreateBuffer(const BufferDescriptor& desc, const void* initialData)
{
    /* Validate and store format size (if supported) */
    std::uint32_t formatSize = 0;

    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        ValidateBufferDesc(desc, &formatSize);
    }

    /* Create buffer object */
    auto bufferDbg = MakeUnique<DbgBuffer>(*instance_->CreateBuffer(desc, initialData), desc);

    /* Store settings */
    bufferDbg->elements     = (formatSize > 0 ? desc.size / formatSize : 0);
    bufferDbg->initialized  = (initialData != nullptr);

    return TakeOwnership(buffers_, std::move(bufferDbg));
}

BufferArray* DbgRenderSystem::CreateBufferArray(std::uint32_t numBuffers, Buffer* const * bufferArray)
{
    AssertCreateBufferArray(numBuffers, bufferArray);

    const auto bindFlags = (*bufferArray)->GetBindFlags();

    /* Create temporary buffer array with buffer instances */
    std::vector<Buffer*>    bufferInstanceArray(numBuffers);
    std::vector<DbgBuffer*> bufferDbgArray(numBuffers);

    for (std::uint32_t i = 0; i < numBuffers; ++i)
    {
        auto bufferDbg          = LLGL_CAST(DbgBuffer*, (*(bufferArray++)));
        bufferInstanceArray[i]  = &(bufferDbg->instance);
        bufferDbgArray[i]       = bufferDbg;
    }

    /* Create native buffer and debug buffer */
    auto bufferArrayInstance    = instance_->CreateBufferArray(numBuffers, bufferInstanceArray.data());
    auto bufferArrayDbg         = MakeUnique<DbgBufferArray>(*bufferArrayInstance, bindFlags, std::move(bufferDbgArray));

    return TakeOwnership(bufferArrays_, std::move(bufferArrayDbg));
}

void DbgRenderSystem::Release(Buffer& buffer)
{
    ReleaseDbg(buffers_, buffer);
}

void DbgRenderSystem::Release(BufferArray& bufferArray)
{
    ReleaseDbg(bufferArrays_, bufferArray);
}

void DbgRenderSystem::WriteBuffer(Buffer& dstBuffer, std::uint64_t dstOffset, const void* data, std::uint64_t dataSize)
{
    auto& dstBufferDbg = LLGL_CAST(DbgBuffer&, dstBuffer);

    if (debugger_)
    {
        LLGL_DBG_SOURCE;

        /* Make a rough approximation if the buffer is now being initialized */
        if (!dstBufferDbg.initialized)
        {
            if (dstOffset == 0)
                dstBufferDbg.initialized = true;
        }

        ValidateBufferBoundary(dstBufferDbg.desc.size, dstOffset, dataSize);

        if (!data)
            LLGL_DBG_ERROR(ErrorType::InvalidArgument, "illegal null pointer argument for 'data' parameter");
    }

    instance_->WriteBuffer(dstBufferDbg.instance, dstOffset, data, dataSize);

    if (profiler_)
        profiler_->frameProfile.bufferWrites++;
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
        bufferDbg.mapped = true;

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

    bufferDbg.mapped = false;
}

/* ----- Textures ----- */

Texture* DbgRenderSystem::CreateTexture(const TextureDescriptor& textureDesc, const SrcImageDescriptor* imageDesc)
{
    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        ValidateTextureDesc(textureDesc, imageDesc);
    }
    return TakeOwnership(textures_, MakeUnique<DbgTexture>(*instance_->CreateTexture(textureDesc, imageDesc), textureDesc));
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

Sampler* DbgRenderSystem::CreateSampler(const SamplerDescriptor& desc)
{
    return instance_->CreateSampler(desc);
    //return TakeOwnership(samplers_, MakeUnique<DbgSampler>());
}

void DbgRenderSystem::Release(Sampler& sampler)
{
    instance_->Release(sampler);
    //ReleaseDbg(samplers_, sampler);
}

/* ----- Resource Views ----- */

ResourceHeap* DbgRenderSystem::CreateResourceHeap(const ResourceHeapDescriptor& desc)
{
    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        ValidateResourceHeapDesc(desc);
    }

    /* Create copy of descriptor to pass native renderer object references */
    auto instanceDesc = desc;
    {
        instanceDesc.pipelineLayout = &(LLGL_CAST(DbgPipelineLayout*, desc.pipelineLayout)->instance);

        for (auto& resourceView : instanceDesc.resourceViews)
        {
            if (auto resource = resourceView.resource)
            {
                switch (resource->GetResourceType())
                {
                    case ResourceType::Buffer:
                        resourceView.resource = &(LLGL_CAST(DbgBuffer*, resourceView.resource)->instance);
                        break;
                    case ResourceType::Texture:
                        resourceView.resource = &(LLGL_CAST(DbgTexture*, resourceView.resource)->instance);
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
                LLGL_DBG_ERROR(ErrorType::InvalidArgument, "null pointer passed to <ResourceViewDescriptor>");
        }
    }
    return TakeOwnership(
        resourceHeaps_,
        MakeUnique<DbgResourceHeap>(*instance_->CreateResourceHeap(instanceDesc), desc)
    );
}

void DbgRenderSystem::Release(ResourceHeap& resourceViewHeap)
{
    return instance_->Release(resourceViewHeap);
}

/* ----- Render Passes ----- */

RenderPass* DbgRenderSystem::CreateRenderPass(const RenderPassDescriptor& desc)
{
    return instance_->CreateRenderPass(desc);
}

void DbgRenderSystem::Release(RenderPass& renderPass)
{
    instance_->Release(renderPass);
}

/* ----- Render Targets ----- */

RenderTarget* DbgRenderSystem::CreateRenderTarget(const RenderTargetDescriptor& desc)
{
    LLGL_DBG_SOURCE;

    auto instanceDesc = desc;

    for (auto& attachment : instanceDesc.attachments)
    {
        if (debugger_)
            ValidateAttachmentDesc(attachment);
        if (auto texture = attachment.texture)
        {
            auto textureDbg = LLGL_CAST(DbgTexture*, texture);
            attachment.texture = &(textureDbg->instance);
        }
    }

    return TakeOwnership(
        renderTargets_,
        MakeUnique<DbgRenderTarget>(*instance_->CreateRenderTarget(instanceDesc), debugger_, desc)
    );
}

void DbgRenderSystem::Release(RenderTarget& renderTarget)
{
    ReleaseDbg(renderTargets_, renderTarget);
}

/* ----- Shader ----- */

Shader* DbgRenderSystem::CreateShader(const ShaderDescriptor& desc)
{
    return TakeOwnership(shaders_, MakeUnique<DbgShader>(*instance_->CreateShader(desc), desc));
}

static Shader* GetInstanceShader(Shader* shader)
{
    if (shader)
    {
        auto shaderDbg = LLGL_CAST(DbgShader*, shader);
        return &(shaderDbg->instance);
    }
    return nullptr;
}

ShaderProgram* DbgRenderSystem::CreateShaderProgram(const ShaderProgramDescriptor& desc)
{
    ShaderProgramDescriptor instanceDesc;
    {
        instanceDesc.vertexShader           = GetInstanceShader(desc.vertexShader);
        instanceDesc.tessControlShader      = GetInstanceShader(desc.tessControlShader);
        instanceDesc.tessEvaluationShader   = GetInstanceShader(desc.tessEvaluationShader);
        instanceDesc.geometryShader         = GetInstanceShader(desc.geometryShader);
        instanceDesc.fragmentShader         = GetInstanceShader(desc.fragmentShader);
        instanceDesc.computeShader          = GetInstanceShader(desc.computeShader);
    }
    return TakeOwnership(shaderPrograms_, MakeUnique<DbgShaderProgram>(*instance_->CreateShaderProgram(instanceDesc), debugger_, desc));
}

void DbgRenderSystem::Release(Shader& shader)
{
    ReleaseDbg(shaders_, shader);
}

void DbgRenderSystem::Release(ShaderProgram& shaderProgram)
{
    ReleaseDbg(shaderPrograms_, shaderProgram);
}

/* ----- Pipeline Layouts ----- */

PipelineLayout* DbgRenderSystem::CreatePipelineLayout(const PipelineLayoutDescriptor& desc)
{
    return TakeOwnership(pipelineLayouts_, MakeUnique<DbgPipelineLayout>(*instance_->CreatePipelineLayout(desc), desc));
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

PipelineState* DbgRenderSystem::CreatePipelineState(const GraphicsPipelineDescriptor& desc, std::unique_ptr<Blob>* serializedCache)
{
    LLGL_DBG_SOURCE;

    if (debugger_)
        ValidateGraphicsPipelineDesc(desc);

    if (desc.shaderProgram)
    {
        auto instanceDesc = desc;
        {
            instanceDesc.shaderProgram  = &(LLGL_CAST(const DbgShaderProgram*, desc.shaderProgram)->instance);
            if (desc.pipelineLayout != nullptr)
                instanceDesc.pipelineLayout = &(LLGL_CAST(const DbgPipelineLayout*, desc.pipelineLayout)->instance);
        }
        return TakeOwnership(pipelineStates_, MakeUnique<DbgPipelineState>(*instance_->CreatePipelineState(instanceDesc, serializedCache), desc));
    }
    else
        LLGL_DBG_ERROR(ErrorType::InvalidArgument, "shader program must not be null");

    return nullptr;
}

PipelineState* DbgRenderSystem::CreatePipelineState(const ComputePipelineDescriptor& desc, std::unique_ptr<Blob>* serializedCache)
{
    LLGL_DBG_SOURCE;

    if (desc.shaderProgram)
    {
        auto instanceDesc = desc;
        {
            instanceDesc.shaderProgram  = &(LLGL_CAST(const DbgShaderProgram*, desc.shaderProgram)->instance);
            if (desc.pipelineLayout != nullptr)
                instanceDesc.pipelineLayout = &(LLGL_CAST(const DbgPipelineLayout*, desc.pipelineLayout)->instance);
        }
        return TakeOwnership(pipelineStates_, MakeUnique<DbgPipelineState>(*instance_->CreatePipelineState(instanceDesc, serializedCache), desc));
    }
    else
        LLGL_DBG_ERROR(ErrorType::InvalidArgument, "shader program must not be null");

    return nullptr;
}

void DbgRenderSystem::Release(PipelineState& pipelineState)
{
    ReleaseDbg(pipelineStates_, pipelineState);
}

/* ----- Queries ----- */

QueryHeap* DbgRenderSystem::CreateQueryHeap(const QueryHeapDescriptor& desc)
{
    return TakeOwnership(queryHeaps_, MakeUnique<DbgQueryHeap>(*instance_->CreateQueryHeap(desc), desc));
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


/*
 * ======= Private: =======
 */

void DbgRenderSystem::ValidateBindFlags(long flags)
{
    const long bufferOnlyFlags =
    (
        BindFlags::VertexBuffer         |
        BindFlags::IndexBuffer          |
        BindFlags::ConstantBuffer       |
        BindFlags::StreamOutputBuffer   |
        BindFlags::IndirectBuffer
    );

    const long textureOnlyFlags =
    (
        BindFlags::ColorAttachment          |
        BindFlags::DepthStencilAttachment
    );

    const long validFlags =
    (
        bufferOnlyFlags     |
        textureOnlyFlags    |
        BindFlags::Sampled  |
        BindFlags::Storage  |
        BindFlags::CopySrc  |
        BindFlags::CopyDst
    );

    /* Check for unknown flags */
    if ((flags & (~validFlags)) != 0)
        LLGL_DBG_WARN(WarningType::ImproperArgument, "unknown bind flags specified");

    /* Validate combination of flags */
    if ((flags & bufferOnlyFlags) != 0 && (flags & textureOnlyFlags) != 0)
        LLGL_DBG_ERROR(ErrorType::InvalidArgument, "cannot combine binding flags that are exclusive for buffers and textures");
    if ((flags & BindFlags::ColorAttachment) != 0 && (flags & BindFlags::DepthStencilAttachment) != 0)
        LLGL_DBG_ERROR(ErrorType::InvalidArgument, "resources cannot have color attachment and depth-stencil attachment binding flags at the same time");
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

void DbgRenderSystem::ValidateBufferDesc(const BufferDescriptor& desc, std::uint32_t* formatSizeOut)
{
    /* Validate flags */
    ValidateBindFlags(desc.bindFlags);
    ValidateCPUAccessFlags(desc.cpuAccessFlags, CPUAccessFlags::ReadWrite, "buffer");
    ValidateMiscFlags(desc.miscFlags, (MiscFlags::DynamicUsage | MiscFlags::NoInitialData), "buffer");

    /* Validate (constant-) buffer size */
    if ((desc.bindFlags & BindFlags::ConstantBuffer) != 0)
        ValidateConstantBufferSize(desc.size);
    else
        ValidateBufferSize(desc.size);

    std::uint32_t formatSize = 0;

    if ((desc.bindFlags & BindFlags::VertexBuffer) != 0 && !desc.vertexAttribs.empty())
    {
        /* Validate all vertex attributes have the same binding slot */
        if (desc.vertexAttribs.size() >= 2)
        {
            for (std::size_t i = 0; i + 1 < desc.vertexAttribs.size(); ++i)
                ValidateVertexAttributesForBuffer(desc.vertexAttribs[i], desc.vertexAttribs[i + 1]);
        }

        /* Validate buffer size for specified vertex format */
        formatSize = desc.vertexAttribs.front().stride;
        if (formatSize > 0 && desc.size % formatSize != 0)
            LLGL_DBG_WARN(WarningType::ImproperArgument, "improper vertex buffer size with vertex format of " + std::to_string(formatSize) + " bytes");
    }

    if ((desc.bindFlags & BindFlags::IndexBuffer) != 0 && desc.format != Format::Undefined)
    {
        /* Validate index format */
        if (desc.format != Format::R16UInt &&
            desc.format != Format::R32UInt)
        {
            if (auto formatName = ToString(desc.format))
                LLGL_DBG_ERROR(ErrorType::InvalidArgument, "invalid index buffer format: LLGL::Format::" + std::string(formatName));
            else
                LLGL_DBG_ERROR(ErrorType::InvalidArgument, "unknown index buffer format: 0x" + ToHex(static_cast<std::uint32_t>(desc.format)));
        }

        /* Validate buffer size for specified index format */
        formatSize = GetFormatAttribs(desc.format).bitSize / 8;
        if (formatSize > 0 && desc.size % formatSize != 0)
        {
            LLGL_DBG_WARN(
                WarningType::ImproperArgument,
                "improper index buffer size with index format of " + std::to_string(formatSize) + " bytes"
            );
        }
    }

    if ((desc.bindFlags & BindFlags::ConstantBuffer) != 0)
    {
        /* Validate pack alginemnt of 16 bytes */
        static const std::uint64_t packAlignment = 16;
        if (desc.size % packAlignment != 0)
            LLGL_DBG_WARN(WarningType::ImproperArgument, "constant buffer size is out of pack alignment (alignment is 16 bytes)");
    }

    /* Validate buffer stride */
    if (desc.stride > 0 && desc.size % desc.stride != 0)
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
    if (dataSize + dstOffset > bufferSize)
        LLGL_DBG_ERROR(ErrorType::InvalidArgument, "buffer size and offset out of bounds");
}

void DbgRenderSystem::ValidateBufferMapping(DbgBuffer& bufferDbg, bool mapMemory)
{
    if (mapMemory)
    {
        if (bufferDbg.mapped)
            LLGL_DBG_ERROR(ErrorType::InvalidState, "cannot map buffer that has already been mapped to CPU local memory");
    }
    else
    {
        if (!bufferDbg.mapped)
            LLGL_DBG_ERROR(ErrorType::InvalidState, "cannot unmap buffer that was not previously mapped to CPU local memory");
    }
}

void DbgRenderSystem::ValidateTextureDesc(const TextureDescriptor& desc, const SrcImageDescriptor* imageDesc)
{
    switch (desc.type)
    {
        case TextureType::Texture1D:
            Validate1DTextureSize(desc.extent.width);
            ValidateTextureSizeDefault(desc.extent.height);
            ValidateTextureSizeDefault(desc.extent.depth);
            break;

        case TextureType::Texture2D:
            Validate2DTextureSize(desc.extent.width);
            Validate2DTextureSize(desc.extent.height);
            ValidateTextureSizeDefault(desc.extent.depth);
            break;

        case TextureType::TextureCube:
            AssertCubeTextures();
            ValidateCubeTextureSize(desc.extent.width, desc.extent.height);
            ValidateTextureSizeDefault(desc.extent.depth);
            break;

        case TextureType::Texture3D:
            Assert3DTextures();
            Validate3DTextureSize(desc.extent.width);
            Validate3DTextureSize(desc.extent.height);
            Validate3DTextureSize(desc.extent.depth);
            break;

        case TextureType::Texture1DArray:
            AssertArrayTextures();
            Validate1DTextureSize(desc.extent.width);
            ValidateTextureSizeDefault(desc.extent.height);
            ValidateTextureSizeDefault(desc.extent.depth);
            break;

        case TextureType::Texture2DArray:
            AssertArrayTextures();
            Validate1DTextureSize(desc.extent.width);
            Validate1DTextureSize(desc.extent.height);
            ValidateTextureSizeDefault(desc.extent.depth);
            break;

        case TextureType::TextureCubeArray:
            AssertCubeArrayTextures();
            ValidateCubeTextureSize(desc.extent.width, desc.extent.height);
            ValidateTextureSizeDefault(desc.extent.depth);
            break;

        case TextureType::Texture2DMS:
            AssertMultiSampleTextures();
            Validate2DTextureSize(desc.extent.width);
            Validate2DTextureSize(desc.extent.height);
            ValidateTextureSizeDefault(desc.extent.depth);
            break;

        case TextureType::Texture2DMSArray:
            AssertMultiSampleTextures();
            AssertArrayTextures();
            Validate2DTextureSize(desc.extent.width);
            Validate2DTextureSize(desc.extent.height);
            ValidateTextureSizeDefault(desc.extent.depth);
            break;

        default:
            LLGL_DBG_ERROR(ErrorType::InvalidArgument, "invalid texture type");
            break;
    }

    ValidateTextureFormatSupported(desc.format);
    ValidateTextureDescMipLevels(desc);
    ValidateArrayTextureLayers(desc.type, desc.arrayLayers);
    ValidateBindFlags(desc.bindFlags);
    ValidateMiscFlags(desc.miscFlags, (MiscFlags::DynamicUsage | MiscFlags::FixedSamples | MiscFlags::GenerateMips | MiscFlags::NoInitialData), "texture");

    /* Check if MIP-map generation is requested  */
    if ((desc.miscFlags & MiscFlags::GenerateMips) != 0)
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
        else if ((desc.miscFlags & MiscFlags::NoInitialData) != 0)
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

void DbgRenderSystem::ValidateTextureDescMipLevels(const TextureDescriptor& desc)
{
    if (desc.mipLevels > 1)
    {
        /* Get number of levels for full MIP-chain */
        auto tempDesc = desc;
        tempDesc.mipLevels = 0;
        auto maxNumMipLevels = NumMipLevels(tempDesc);

        if (desc.mipLevels > maxNumMipLevels)
        {
            LLGL_DBG_ERROR(
                ErrorType::InvalidArgument,
                "number of MIP-map levels exceeded limit (" + std::to_string(desc.mipLevels) +
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
    const auto& subresource         = textureRegion.subresource;
    const auto  baseSubresource     = TextureSubresource{ 0, subresource.numArrayLayers, 0, subresource.numMipLevels };
    const auto  numTexels           = NumMipTexels(textureDbg.desc.type, textureRegion.extent, baseSubresource);
    const auto  requiredDataSize    = GetMemoryFootprint(imageFormat, dataType, numTexels);

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

void DbgRenderSystem::ValidateTextureView(const DbgTexture& sharedTextureDbg, const TextureViewDescriptor& desc)
{
    /* Validate texture-view features are supported */
    if (!GetRenderingCaps().features.hasTextureViews)
        LLGL_DBG_ERROR(ErrorType::UnsupportedFeature, "texture views not supported");
    if (!GetRenderingCaps().features.hasTextureViewSwizzle && !IsTextureSwizzleIdentity(desc.swizzle))
        LLGL_DBG_ERROR(ErrorType::UnsupportedFeature, "texture view swizzle not supported, but mapping is not equal to identity");

    /* Validate attributes of shared texture against texture-view descriptor */
    if (sharedTextureDbg.isTextureView)
        LLGL_DBG_ERROR(ErrorType::InvalidArgument, "texture view cannot be shared with another texture view");

    const auto mipLevelUpperBound = desc.subresource.baseMipLevel + desc.subresource.numMipLevels;
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
    const auto dstType = desc.type;

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

void DbgRenderSystem::ValidateAttachmentDesc(const AttachmentDescriptor& desc)
{
    if (auto texture = desc.texture)
    {
        auto textureDbg = LLGL_CAST(DbgTexture*, texture);

        /* Validate attachment type for this texture */
        if (desc.type == AttachmentType::Color)
        {
            if ((textureDbg->desc.bindFlags & BindFlags::ColorAttachment) == 0)
            {
                LLGL_DBG_ERROR(
                    ErrorType::InvalidState,
                    "cannot have color attachment with a texture that was not created with the 'LLGL::BindFlags::ColorAttachment' flag"
                );
            }
        }
        else
        {
            if ((textureDbg->desc.bindFlags & BindFlags::DepthStencilAttachment) == 0)
            {
                LLGL_DBG_ERROR(
                    ErrorType::InvalidState,
                    "cannot have depth-stencil attachment with a texture that was not created with the 'LLGL::BindFlags::DepthStencilAttachment' flag"
                );
            }
        }

        /* Validate MIP-level */
        if (desc.mipLevel >= textureDbg->mipLevels)
        {
            LLGL_DBG_ERROR(
                ErrorType::InvalidArgument,
                "render-target attachment exceeded number of MIP-map levels (" +
                std::to_string(desc.mipLevel) + " specified but upper bound is " + std::to_string(textureDbg->mipLevels) + ")"
            );
        }

        /* Validate array layer */
        if (desc.arrayLayer >= textureDbg->desc.arrayLayers)
        {
            LLGL_DBG_ERROR(
                ErrorType::InvalidArgument,
                "render-target attachment exceeded number of array layers (" +
                std::to_string(desc.arrayLayer) + " specified but upper bound is " + std::to_string(textureDbg->desc.arrayLayers) + ")"
            );
        }
    }
    else
    {
        if (desc.type == AttachmentType::Color)
        {
            LLGL_DBG_ERROR(
                ErrorType::InvalidArgument,
                "cannot have color attachment with 'texture' member being a null pointer"
            );
        }
    }
}

void DbgRenderSystem::ValidateResourceHeapDesc(const ResourceHeapDescriptor& desc)
{
    if (desc.pipelineLayout)
    {
        auto pipelineLayoutDbg = LLGL_CAST(DbgPipelineLayout*, desc.pipelineLayout);
        const auto& bindings = pipelineLayoutDbg->desc.bindings;

        const auto numResourceViews = desc.resourceViews.size();
        const auto numBindings      = bindings.size();

        if (numBindings == 0)
            LLGL_DBG_ERROR(ErrorType::InvalidArgument, "cannot create resource heap with empty pipeline layout");
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
        else
        {
            /* Validate all resource view descriptors against their respective binding descriptor */
            for (std::size_t i = 0, n = desc.resourceViews.size(), m = bindings.size(); i < n; ++i)
                ValidateResourceViewForBinding(desc.resourceViews[i], bindings[i % m]);
        }
    }
    else
        LLGL_DBG_ERROR(ErrorType::InvalidArgument, "pipeline layout must not be null");
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
                //if (IsBufferViewEnabled(rvDesc.bufferView))
                //    ValidateBufferView(*bufferDbg, rvDesc.bufferView);
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
            "binding flags mismatch between buffer resource (slot = " +
            std::to_string(bindingDesc.slot) + ") and binding descriptor"
        );
    }
}

void DbgRenderSystem::ValidateTextureForBinding(const DbgTexture& textureDbg, const BindingDescriptor& bindingDesc)
{
    if ((textureDbg.desc.bindFlags & bindingDesc.bindFlags) != bindingDesc.bindFlags)
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "binding flags mismatch between texture resource (slot = " +
            std::to_string(bindingDesc.slot) + ") and binding descriptor"
        );
    }
}

// Converts the specified color mask into a string representation (e.g. "RGBA" or "R_G_").
static std::string ColorMaskToString(const ColorRGBAb& color)
{
    std::string s;

    s += (color.r ? 'R' : '_');
    s += (color.g ? 'G' : '_');
    s += (color.b ? 'B' : '_');
    s += (color.a ? 'A' : '_');

    return s;
}

void DbgRenderSystem::ValidateColorMaskIsDisabled(const BlendTargetDescriptor& desc, std::size_t idx)
{
    if (desc.colorMask.r || desc.colorMask.g || desc.colorMask.b || desc.colorMask.a)
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "cannot use color mask <" + ColorMaskToString(desc.colorMask) +
            "> of blend target <" + std::to_string(idx) + "> without a fragment shader"
        );
    }
}

void DbgRenderSystem::ValidateBlendDescriptor(const BlendDescriptor& desc, bool hasFragmentShader)
{
    /* Validate proper use of logic pixel operations */
    if (desc.logicOp != LogicOp::Disabled)
    {
        if (!GetRenderingCaps().features.hasLogicOp)
            LLGL_DBG_ERROR(ErrorType::UnsupportedFeature, "logic pixel operations not supported");

        if (desc.independentBlendEnabled)
        {
            LLGL_DBG_ERROR(
                ErrorType::InvalidArgument,
                "logic pixel operations cannot be used in combination with independent blending"
            );
        }

        for (const auto& target : desc.targets)
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
        if (desc.independentBlendEnabled)
        {
            for (std::size_t i = 0; i < sizeof(desc.targets)/sizeof(desc.targets[0]); ++i)
                ValidateColorMaskIsDisabled(desc.targets[i], i);
        }
        else
            ValidateColorMaskIsDisabled(desc.targets[0], 0);
    }
}

void DbgRenderSystem::ValidateGraphicsPipelineDesc(const GraphicsPipelineDescriptor& desc)
{
    if (desc.rasterizer.conservativeRasterization && !features_.hasConservativeRasterization)
        LLGL_DBG_ERROR_NOT_SUPPORTED("conservative rasterization");

    /* Determine if the shader program contains a fragment shader */
    bool hasFragmentShader = false;
    if (desc.shaderProgram)
    {
        auto shaderProgramDbg = LLGL_CAST(const DbgShaderProgram*, desc.shaderProgram);
        hasFragmentShader = shaderProgramDbg->HasFragmentShader();
    }

    ValidateBlendDescriptor(desc.blend, hasFragmentShader);
    ValidatePrimitiveTopology(desc.primitiveTopology);
}

void DbgRenderSystem::ValidatePrimitiveTopology(const PrimitiveTopology primitiveTopology)
{
    switch (primitiveTopology)
    {
        case PrimitiveTopology::LineLoop:
            if (GetRendererID() != RendererID::OpenGL)
                LLGL_DBG_ERROR_NOT_SUPPORTED("primitive topology 'LLGL::PrimitiveTopology::LineLoop'");
            break;
        case PrimitiveTopology::TriangleFan:
            if (GetRendererID() != RendererID::OpenGL && GetRendererID() != RendererID::Vulkan)
                LLGL_DBG_ERROR_NOT_SUPPORTED("primitive topology 'LLGL::PrimitiveTopology::TriangleFan'");
            break;
        default:
            break;
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
void DbgRenderSystem::ReleaseDbg(std::set<std::unique_ptr<T>>& cont, TBase& entry)
{
    auto& entryDbg = LLGL_CAST(T&, entry);
    instance_->Release(entryDbg.instance);
    RemoveFromUniqueSet(cont, &entry);
}


} // /namespace LLGL



// ================================================================================
