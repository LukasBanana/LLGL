/*
 * DbgRenderSystem.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "DbgRenderSystem.h"
#include "DbgCore.h"
#include "../../Core/Helper.h"
#include "../CheckedCast.h"


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
    const std::shared_ptr<RenderSystem>& instance, RenderingProfiler* profiler, RenderingDebugger* debugger) :
        instance_ { instance           },
        profiler_ { profiler           },
        debugger_ { debugger           },
        caps_     { GetRenderingCaps() },
        features_ { caps_.features     },
        limits_   { caps_.limits       }
{
}

DbgRenderSystem::~DbgRenderSystem()
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
    auto renderContextInstance = instance_->CreateRenderContext(desc, surface);

    SetRendererInfo(instance_->GetRendererInfo());
    SetRenderingCaps(instance_->GetRenderingCaps());

    return TakeOwnership(renderContexts_, MakeUnique<DbgRenderContext>(*renderContextInstance));
}

void DbgRenderSystem::Release(RenderContext& renderContext)
{
    ReleaseDbg(renderContexts_, renderContext);
}

/* ----- Command queues ----- */

CommandQueue* DbgRenderSystem::GetCommandQueue()
{
    return instance_->GetCommandQueue();
}

/* ----- Command buffers ----- */

CommandBuffer* DbgRenderSystem::CreateCommandBuffer()
{
    return TakeOwnership(commandBuffers_, MakeUnique<DbgCommandBuffer>(
        *instance_->CreateCommandBuffer(), nullptr, profiler_, debugger_, GetRenderingCaps()
    ));
}

CommandBufferExt* DbgRenderSystem::CreateCommandBufferExt()
{
    if (auto instance = instance_->CreateCommandBufferExt())
    {
        return TakeOwnership(commandBuffers_, MakeUnique<DbgCommandBuffer>(
            *instance, instance, profiler_, debugger_, GetRenderingCaps()
        ));
    }
    return nullptr;
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
    auto bufferDbg = MakeUnique<DbgBuffer>(*instance_->CreateBuffer(desc, initialData), desc.type);

    /* Store settings */
    bufferDbg->desc         = desc;
    bufferDbg->elements     = (formatSize > 0 ? desc.size / formatSize : 0);
    bufferDbg->initialized  = (initialData != nullptr);

    return TakeOwnership(buffers_, std::move(bufferDbg));
}

BufferArray* DbgRenderSystem::CreateBufferArray(std::uint32_t numBuffers, Buffer* const * bufferArray)
{
    AssertCreateBufferArray(numBuffers, bufferArray);

    const auto bufferType = (*bufferArray)->GetType();

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
    auto bufferArrayDbg         = MakeUnique<DbgBufferArray>(*bufferArrayInstance, bufferType);

    /* Store buffer references */
    bufferArrayDbg->buffers = std::move(bufferDbgArray);

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

void DbgRenderSystem::WriteBuffer(Buffer& buffer, const void* data, std::size_t dataSize, std::size_t offset)
{
    auto& bufferDbg = LLGL_CAST(DbgBuffer&, buffer);

    if (debugger_)
    {
        LLGL_DBG_SOURCE;

        /* Make a rough approximation if the buffer is now being initialized */
        if (!bufferDbg.initialized)
        {
            if (offset == 0)
                bufferDbg.initialized = true;
        }

        ValidateBufferBoundary(bufferDbg.desc.size, dataSize, offset);

        if (!data)
            LLGL_DBG_ERROR(ErrorType::InvalidArgument, "illegal null pointer argument for 'data' parameter");
    }

    instance_->WriteBuffer(bufferDbg.instance, data, dataSize, offset);

    LLGL_DBG_PROFILER_DO(writeBuffer.Inc());
}

void* DbgRenderSystem::MapBuffer(Buffer& buffer, const CPUAccess access)
{
    void* result = nullptr;
    auto& bufferDbg = LLGL_CAST(DbgBuffer&, buffer);

    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        ValidateBufferCPUAccess(bufferDbg, access);
        ValidateBufferMapping(bufferDbg, true);
    }

    result = instance_->MapBuffer(bufferDbg.instance, access);

    bufferDbg.mapped = true;

    LLGL_DBG_PROFILER_DO(mapBuffer.Inc());
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
        ValidateTextureDesc(textureDesc);
    }
    return TakeOwnership(textures_, MakeUnique<DbgTexture>(*instance_->CreateTexture(textureDesc, imageDesc), textureDesc));
}

void DbgRenderSystem::Release(Texture& texture)
{
    ReleaseDbg(textures_, texture);
}

void DbgRenderSystem::WriteTexture(Texture& texture, const SubTextureDescriptor& subTextureDesc, const SrcImageDescriptor& imageDesc)
{
    auto& textureDbg = LLGL_CAST(DbgTexture&, texture);

    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        ValidateMipLevelLimit(subTextureDesc.mipLevel, textureDbg.mipLevels);
    }

    instance_->WriteTexture(textureDbg.instance, subTextureDesc, imageDesc);
}

void DbgRenderSystem::ReadTexture(const Texture& texture, std::uint32_t mipLevel, const DstImageDescriptor& imageDesc)
{
    auto& textureDbg = LLGL_CAST(const DbgTexture&, texture);

    if (debugger_)
    {
        LLGL_DBG_SOURCE;

        /* Validate MIP-level */
        ValidateMipLevelLimit(mipLevel, textureDbg.mipLevels);

        /* Validate output data size */
        const auto requiredDataSize =
        (
            textureDbg.desc.extent.width        *
            textureDbg.desc.extent.height       *
            textureDbg.desc.extent.depth        *
            textureDbg.desc.arrayLayers         *
            ImageFormatSize(imageDesc.format)   *
            DataTypeSize(imageDesc.dataType)
        );

        ValidateTextureImageDataSize(imageDesc.dataSize, requiredDataSize);
    }

    instance_->ReadTexture(textureDbg.instance, mipLevel, imageDesc);
}

void DbgRenderSystem::GenerateMips(Texture& texture)
{
    auto& textureDbg = LLGL_CAST(DbgTexture&, texture);

    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        if (ValidateTextureMips(textureDbg))
            ValidateTextureMipRange(textureDbg, 0, textureDbg.mipLevels);
    }

    instance_->GenerateMips(textureDbg.instance);
}

void DbgRenderSystem::GenerateMips(Texture& texture, std::uint32_t baseMipLevel, std::uint32_t numMipLevels, std::uint32_t baseArrayLayer, std::uint32_t numArrayLayers)
{
    auto& textureDbg = LLGL_CAST(DbgTexture&, texture);

    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        if (ValidateTextureMips(textureDbg))
        {
            ValidateTextureMipRange(textureDbg, baseMipLevel, numMipLevels);
            ValidateTextureArrayRange(textureDbg, baseArrayLayer, numArrayLayers);
        }
    }

    instance_->GenerateMips(textureDbg.instance, baseMipLevel, numMipLevels, baseArrayLayer, numArrayLayers);
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
    //RemoveFromUniqueSet(samplers_, &sampler);
}

/* ----- Resource Views ----- */

ResourceHeap* DbgRenderSystem::CreateResourceHeap(const ResourceHeapDescriptor& desc)
{
    auto instanceDesc = desc;
    {
        for (auto& resourceView : instanceDesc.resourceViews)
        {
            if (auto resource = resourceView.resource)
            {
                switch (resource->QueryResourceType())
                {
                    case ResourceType::VertexBuffer:
                    case ResourceType::IndexBuffer:
                    case ResourceType::ConstantBuffer:
                    case ResourceType::StorageBuffer:
                    case ResourceType::StreamOutputBuffer:
                        resourceView.resource = &(LLGL_CAST(DbgBuffer*, resourceView.resource)->instance);
                        break;
                    case ResourceType::Texture:
                        resourceView.resource = &(LLGL_CAST(DbgTexture*, resourceView.resource)->instance);
                        break;
                    case ResourceType::Sampler:
                        //TODO: DbgSampler
                        break;
                    default:
                        LLGL_DBG_ERROR(ErrorType::InvalidArgument, "invalid resource type passed to ResourceViewDescriptor");
                        break;
                }
            }
            else
                LLGL_DBG_ERROR(ErrorType::InvalidArgument, "null pointer passed to ResourceViewDescriptor");
        }
    }
    return instance_->CreateResourceHeap(instanceDesc);
}

void DbgRenderSystem::Release(ResourceHeap& resourceViewHeap)
{
    return instance_->Release(resourceViewHeap);
}

/* ----- Render Targets ----- */

RenderTarget* DbgRenderSystem::CreateRenderTarget(const RenderTargetDescriptor& desc)
{
    auto instanceDesc = desc;

    for (auto& attachment : instanceDesc.attachments)
    {
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
    RemoveFromUniqueSet(renderTargets_, &renderTarget);
}

/* ----- Shader ----- */

Shader* DbgRenderSystem::CreateShader(const ShaderDescriptor& desc)
{
    return TakeOwnership(shaders_, MakeUnique<DbgShader>(*instance_->CreateShader(desc), desc.type, debugger_));
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
        instanceDesc.vertexFormats          = desc.vertexFormats;
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
    return instance_->CreatePipelineLayout(desc);
}

void DbgRenderSystem::Release(PipelineLayout& pipelineLayout)
{
    instance_->Release(pipelineLayout);
}

/* ----- Pipeline States ----- */

GraphicsPipeline* DbgRenderSystem::CreateGraphicsPipeline(const GraphicsPipelineDescriptor& desc)
{
    LLGL_DBG_SOURCE;

    if (debugger_)
        ValidateGraphicsPipelineDesc(desc);

    if (desc.shaderProgram)
    {
        auto instanceDesc = desc;
        {
            auto shaderProgramDbg = LLGL_CAST(DbgShaderProgram*, desc.shaderProgram);
            instanceDesc.shaderProgram = &(shaderProgramDbg->instance);

            if (desc.renderTarget)
            {
                auto renderTargetDbg = LLGL_CAST(DbgRenderTarget*, desc.renderTarget);
                instanceDesc.renderTarget = &(renderTargetDbg->instance);
            }
        }
        return TakeOwnership(graphicsPipelines_, MakeUnique<DbgGraphicsPipeline>(*instance_->CreateGraphicsPipeline(instanceDesc), desc));
    }
    else
        LLGL_DBG_ERROR(ErrorType::InvalidArgument, "shader program must not be null");

    return nullptr;
}

ComputePipeline* DbgRenderSystem::CreateComputePipeline(const ComputePipelineDescriptor& desc)
{
    if (desc.shaderProgram)
    {
        auto instanceDesc = desc;
        {
            auto shaderProgramDbg = LLGL_CAST(DbgShaderProgram*, desc.shaderProgram);
            instanceDesc.shaderProgram = &(shaderProgramDbg->instance);
        }
        return instance_->CreateComputePipeline(instanceDesc);
    }
    else
        LLGL_DBG_ERROR(ErrorType::InvalidArgument, "shader program must not be null");

    return nullptr;
}

void DbgRenderSystem::Release(GraphicsPipeline& graphicsPipeline)
{
    ReleaseDbg(graphicsPipelines_, graphicsPipeline);
}

void DbgRenderSystem::Release(ComputePipeline& computePipeline)
{
    instance_->Release(computePipeline);
    //RemoveFromUniqueSet(computePipelines_, &computePipeline);
}

/* ----- Queries ----- */

Query* DbgRenderSystem::CreateQuery(const QueryDescriptor& desc)
{
    return TakeOwnership(queries_, MakeUnique<DbgQuery>(*instance_->CreateQuery(desc), desc));
}

void DbgRenderSystem::Release(Query& query)
{
    ReleaseDbg(queries_, query);
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

void DbgRenderSystem::ValidateBufferDesc(const BufferDescriptor& desc, std::uint32_t* formatSize)
{
    /* Validate (constant-) buffer size */
    if (desc.type == BufferType::Constant)
        ValidateConstantBufferSize(desc.size);
    else
        ValidateBufferSize(desc.size);

    std::uint32_t formatSizeTemp = 0;

    switch (desc.type)
    {
        case BufferType::Vertex:
        {
            /* Validate buffer size for specified vertex format */
            formatSizeTemp = desc.vertexBuffer.format.stride;
            if (formatSizeTemp > 0 && desc.size % formatSizeTemp != 0)
                LLGL_DBG_WARN(WarningType::ImproperArgument, "improper vertex buffer size with vertex format of " + std::to_string(formatSizeTemp) + " bytes");
        }
        break;

        case BufferType::Index:
        {
            /* Validate buffer size for specified index format */
            formatSizeTemp = desc.indexBuffer.format.GetFormatSize();
            if (formatSizeTemp > 0 && desc.size % formatSizeTemp != 0)
                LLGL_DBG_WARN(WarningType::ImproperArgument, "improper index buffer size with index format of " + std::to_string(formatSizeTemp) + " bytes");
        }
        break;

        case BufferType::Constant:
        {
            /* Validate pack alginemnt of 16 bytes */
            static const std::size_t packAlignment = 16;
            if (desc.size % packAlignment != 0)
                LLGL_DBG_WARN(WarningType::ImproperArgument, "constant buffer size is out of pack alignment (alignment is 16 bytes)");
        }
        break;

        default:
        break;
    }

    if (formatSize)
        *formatSize = formatSizeTemp;
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

void DbgRenderSystem::ValidateBufferBoundary(std::uint64_t bufferSize, std::size_t dataSize, std::size_t dataOffset)
{
    if (static_cast<std::uint64_t>(dataSize) + static_cast<std::uint64_t>(dataOffset) > bufferSize)
        LLGL_DBG_ERROR(ErrorType::InvalidArgument, "buffer size and offset out of bounds");
}

void DbgRenderSystem::ValidateBufferCPUAccess(DbgBuffer& bufferDbg, const CPUAccess access)
{
    if (access == CPUAccess::ReadOnly || access == CPUAccess::ReadWrite)
    {
        if ((bufferDbg.desc.flags & BufferFlags::MapReadAccess) == 0)
            LLGL_DBG_ERROR(ErrorType::InvalidState, "cannot map buffer with CPU read access (buffer was not created with 'LLGL::BufferFlags::MapReadAccess' flag)");
    }
    if (access == CPUAccess::WriteOnly || access == CPUAccess::ReadWrite)
    {
        if ((bufferDbg.desc.flags & BufferFlags::MapWriteAccess) == 0)
            LLGL_DBG_ERROR(ErrorType::InvalidState, "cannot map buffer with CPU write access (buffer was not created with 'LLGL::BufferFlags::MapWriteAccess' flag)");
    }
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

void DbgRenderSystem::ValidateTextureDesc(const TextureDescriptor& desc)
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

    ValidateTextureDescMipLevels(desc);
    ValidateArrayTextureLayers(desc.type, desc.arrayLayers);
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
                        "number fo texture layers must be 6 for cube textures (but " +
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
                        "number fo texture layers must be a multiple of 6 for cube array textures (but " +
                        std::to_string(layers) + " was specified)"
                    );
                }
            }
            break;

            default:
            {
                if (IsArrayTexture(type))
                {
                    const auto maxNumLayers = limits_.maxNumTextureArrayLayers;
                    if (layers > maxNumLayers)
                    {
                        LLGL_DBG_ERROR(
                            ErrorType::InvalidArgument,
                            "number fo texture layers exceeded limit (" + std::to_string(layers) +
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

void DbgRenderSystem::ValidateMipLevelLimit(std::uint32_t mipLevel, std::uint32_t mipLevelCount)
{
    if (mipLevel >= mipLevelCount)
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "mip level out of bounds (" + std::to_string(mipLevel) +
            " specified but limit is " + std::to_string(mipLevelCount > 0 ? mipLevelCount - 1 : 0) + ")"
        );
    }
}

void DbgRenderSystem::ValidateTextureImageDataSize(std::size_t dataSize, std::size_t requiredDataSize)
{
    if (dataSize < requiredDataSize)
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "image data size too small for texture (" + std::to_string(dataSize) +
            " specified but required is " + std::to_string(requiredDataSize) + ")"
        );
    }
}

bool DbgRenderSystem::ValidateTextureMips(const DbgTexture& textureDbg)
{
    if (textureDbg.desc.mipLevels == 1)
    {
        LLGL_DBG_ERROR(ErrorType::InvalidArgument, "cannot generate MIP-maps for texture with only one MIP-map level");
        return false;
    }
    return true;
}

void DbgRenderSystem::ValidateTextureMipRange(const DbgTexture& textureDbg, std::uint32_t baseMipLevel, std::uint32_t numMipLevels)
{
    const auto mipLevelRangeEnd = baseMipLevel + numMipLevels;
    if (mipLevelRangeEnd > textureDbg.mipLevels)
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "MIP level out of range for texture (" + std::to_string(mipLevelRangeEnd) +
            " specified but limit is " + std::to_string(textureDbg.mipLevels) + ")"
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

void DbgRenderSystem::ValidateGraphicsPipelineDesc(const GraphicsPipelineDescriptor& desc)
{
    if (desc.rasterizer.conservativeRasterization && !features_.hasConservativeRasterization)
        LLGL_DBG_ERROR_NOT_SUPPORTED("conservative rasterization");
    if (desc.blend.targets.size() > 8)
        LLGL_DBG_ERROR(ErrorType::InvalidArgument, "too many blend state targets (limit is 8)");

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
