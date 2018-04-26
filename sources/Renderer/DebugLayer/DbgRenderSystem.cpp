/*
 * DbgRenderSystem.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
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
        instance_ { instance },
        profiler_ { profiler },
        debugger_ { debugger }
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
        *instance_->CreateCommandBuffer(), profiler_, debugger_, GetRenderingCaps()
    ));
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

        switch (desc.type)
        {
            case BufferType::Vertex:
            {
                /* Validate buffer size for specified vertex format */
                formatSize = desc.vertexBuffer.format.stride;
                if (formatSize > 0 && desc.size % formatSize != 0)
                    LLGL_DBG_WARN(WarningType::ImproperArgument, "improper vertex buffer size with vertex format of " + std::to_string(formatSize) + " bytes");
            }
            break;

            case BufferType::Index:
            {
                /* Validate buffer size for specified index format */
                formatSize = desc.indexBuffer.format.GetFormatSize();
                if (formatSize > 0 && desc.size % formatSize != 0)
                    LLGL_DBG_WARN(WarningType::ImproperArgument, "improper index buffer size with index format of " + std::to_string(formatSize) + " bytes");
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

        DebugBufferSize(bufferDbg.desc.size, dataSize, offset);
    }
    
    instance_->WriteBuffer(bufferDbg.instance, data, dataSize, offset);
    
    LLGL_DBG_PROFILER_DO(writeBuffer.Inc());
}

void* DbgRenderSystem::MapBuffer(Buffer& buffer, const BufferCPUAccess access)
{
    void* result = nullptr;
    auto& bufferDbg = LLGL_CAST(DbgBuffer&, buffer);
    {
        result = instance_->MapBuffer(bufferDbg.instance, access);
    }
    LLGL_DBG_PROFILER_DO(mapBuffer.Inc());
    return result;
}

void DbgRenderSystem::UnmapBuffer(Buffer& buffer)
{
    auto& bufferDbg = LLGL_CAST(DbgBuffer&, buffer);
    instance_->UnmapBuffer(bufferDbg.instance);
}

/* ----- Textures ----- */

Texture* DbgRenderSystem::CreateTexture(const TextureDescriptor& textureDesc, const ImageDescriptor* imageDesc)
{
    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        DebugTextureDescriptor(textureDesc);
    }
    return TakeOwnership(textures_, MakeUnique<DbgTexture>(*instance_->CreateTexture(textureDesc, imageDesc), textureDesc));
}

TextureArray* DbgRenderSystem::CreateTextureArray(std::uint32_t numTextures, Texture* const * textureArray)
{
    AssertCreateTextureArray(numTextures, textureArray);

    /* Create temporary buffer array with buffer instances */
    std::vector<Texture*> textureInstanceArray;
    for (std::uint32_t i = 0; i < numTextures; ++i)
    {
        auto textureDbg = LLGL_CAST(DbgTexture*, (*(textureArray++)));
        textureInstanceArray.push_back(&(textureDbg->instance));
    }

    return instance_->CreateTextureArray(numTextures, textureInstanceArray.data());
}

void DbgRenderSystem::Release(Texture& texture)
{
    ReleaseDbg(textures_, texture);
}

void DbgRenderSystem::Release(TextureArray& textureArray)
{
    instance_->Release(textureArray);
    //ReleaseDbg(textureArrays_, textureArray);
}

void DbgRenderSystem::WriteTexture(Texture& texture, const SubTextureDescriptor& subTextureDesc, const ImageDescriptor& imageDesc)
{
    auto& textureDbg = LLGL_CAST(DbgTexture&, texture);

    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        DebugMipLevelLimit(subTextureDesc.mipLevel, textureDbg.mipLevels);
    }
    
    instance_->WriteTexture(textureDbg.instance, subTextureDesc, imageDesc);
}

void DbgRenderSystem::ReadTexture(const Texture& texture, std::uint32_t mipLevel, ImageFormat imageFormat, DataType dataType, void* data, std::size_t dataSize)
{
    auto& textureDbg = LLGL_CAST(const DbgTexture&, texture);

    if (debugger_)
    {
        LLGL_DBG_SOURCE;

        /* Validate MIP-level */
        DebugMipLevelLimit(mipLevel, textureDbg.mipLevels);

        /* Validate output data size */
        const auto requiredDataSize =
        (
            textureDbg.desc.texture3D.width *
            textureDbg.desc.texture3D.height *
            textureDbg.desc.texture3D.depth *
            ImageFormatSize(imageFormat) *
            DataTypeSize(dataType)
        );

        DebugTextureImageDataSize(dataSize, requiredDataSize);
    }
    
    instance_->ReadTexture(textureDbg.instance, mipLevel, imageFormat, dataType, data, dataSize);
}

void DbgRenderSystem::GenerateMips(Texture& texture)
{
    auto& textureDbg = LLGL_CAST(DbgTexture&, texture);
    {
        instance_->GenerateMips(textureDbg.instance);
    }
    const auto& tex3DDesc = textureDbg.desc.texture3D;
    textureDbg.mipLevels = NumMipLevels(tex3DDesc.width, tex3DDesc.height, tex3DDesc.depth);
}

/* ----- Sampler States ---- */

Sampler* DbgRenderSystem::CreateSampler(const SamplerDescriptor& desc)
{
    return instance_->CreateSampler(desc);
    //return TakeOwnership(samplers_, MakeUnique<DbgSampler>());
}

SamplerArray* DbgRenderSystem::CreateSamplerArray(std::uint32_t numSamplers, Sampler* const * samplerArray)
{
    AssertCreateSamplerArray(numSamplers, samplerArray);
    return instance_->CreateSamplerArray(numSamplers, samplerArray);
}

void DbgRenderSystem::Release(Sampler& sampler)
{
    instance_->Release(sampler);
    //RemoveFromUniqueSet(samplers_, &sampler);
}

void DbgRenderSystem::Release(SamplerArray& samplerArray)
{
    instance_->Release(samplerArray);
    //RemoveFromUniqueSet(samplerArrays_, &samplerArray);
}

/* ----- Render Targets ----- */

RenderTarget* DbgRenderSystem::CreateRenderTarget(const RenderTargetDescriptor& desc)
{
    return TakeOwnership(renderTargets_, MakeUnique<DbgRenderTarget>(*instance_->CreateRenderTarget(desc), debugger_, desc));
}

void DbgRenderSystem::Release(RenderTarget& renderTarget)
{
    RemoveFromUniqueSet(renderTargets_, &renderTarget);
}

/* ----- Shader ----- */

Shader* DbgRenderSystem::CreateShader(const ShaderType type)
{
    return TakeOwnership(shaders_, MakeUnique<DbgShader>(*instance_->CreateShader(type), type, debugger_));
}

ShaderProgram* DbgRenderSystem::CreateShaderProgram()
{
    return TakeOwnership(shaderPrograms_, MakeUnique<DbgShaderProgram>(*instance_->CreateShaderProgram(), debugger_));
}

void DbgRenderSystem::Release(Shader& shader)
{
    ReleaseDbg(shaders_, shader);
}

void DbgRenderSystem::Release(ShaderProgram& shaderProgram)
{
    ReleaseDbg(shaderPrograms_, shaderProgram);
}

/* ----- Pipeline States ----- */

GraphicsPipeline* DbgRenderSystem::CreateGraphicsPipeline(const GraphicsPipelineDescriptor& desc)
{
    LLGL_DBG_SOURCE;

    if (debugger_)
    {
        if (desc.rasterizer.conservativeRasterization && !GetRenderingCaps().hasConservativeRasterization)
            LLGL_DBG_ERROR_NOT_SUPPORTED("conservative rasterization");
        if (desc.blend.targets.size() > 8)
            LLGL_DBG_ERROR(ErrorType::InvalidArgument, "too many blend state targets (limit is 8)");

        if (GetRendererID() != RendererID::OpenGL)
        {
            switch (desc.primitiveTopology)
            {
                case PrimitiveTopology::LineLoop:
                    LLGL_DBG_ERROR(ErrorType::InvalidArgument, "renderer does not support primitive topology line loop");
                    break;
                case PrimitiveTopology::TriangleFan:
                    LLGL_DBG_ERROR(ErrorType::InvalidArgument, "renderer does not support primitive topology triangle fan");
                    break;
                default:
                    break;
            }
        }
    }

    if (desc.shaderProgram)
    {
        GraphicsPipelineDescriptor instanceDesc = desc;
        {
            auto shaderProgramDbg = LLGL_CAST(DbgShaderProgram*, desc.shaderProgram);
            instanceDesc.shaderProgram = &(shaderProgramDbg->instance);
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
        ComputePipelineDescriptor instanceDesc = desc;
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

void DbgRenderSystem::DebugBufferSize(std::uint64_t bufferSize, std::size_t dataSize, std::size_t dataOffset)
{
    if (static_cast<std::uint64_t>(dataSize) + static_cast<std::uint64_t>(dataOffset) > bufferSize)
        LLGL_DBG_ERROR(ErrorType::InvalidArgument, "buffer size and offset out of bounds");
}

void DbgRenderSystem::DebugMipLevelLimit(std::uint32_t mipLevel, std::uint32_t mipLevelCount)
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

void DbgRenderSystem::DebugTextureImageDataSize(std::uint32_t dataSize, std::uint32_t requiredDataSize)
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

void DbgRenderSystem::DebugTextureDescriptor(const TextureDescriptor& desc)
{
    switch (desc.type)
    {
        case TextureType::Texture1D:
            DebugTextureSize(desc.texture1D.width);
            if (desc.texture1D.layers > 1)
                WarnTextureLayersGreaterOne();
            break;

        case TextureType::Texture2D:
        case TextureType::TextureCube:
            DebugTextureSize(desc.texture2D.width);
            DebugTextureSize(desc.texture2D.height);
            if (desc.texture2D.layers > 1)
                WarnTextureLayersGreaterOne();
            break;

        case TextureType::Texture3D:
            DebugTextureSize(desc.texture3D.width);
            DebugTextureSize(desc.texture3D.height);
            DebugTextureSize(desc.texture3D.depth);
            break;

        case TextureType::Texture1DArray:
            DebugTextureSize(desc.texture1D.width);
            if (desc.texture1D.layers == 0)
                ErrTextureLayersEqualZero();
            break;

        case TextureType::Texture2DArray:
        case TextureType::TextureCubeArray:
            DebugTextureSize(desc.texture2D.width);
            DebugTextureSize(desc.texture2D.height);
            if (desc.texture2D.layers == 0)
                ErrTextureLayersEqualZero();
            break;

        case TextureType::Texture2DMS:
            DebugTextureSize(desc.texture2DMS.width);
            DebugTextureSize(desc.texture2DMS.height);
            if (desc.texture2DMS.layers > 1)
                WarnTextureLayersGreaterOne();
            break;

        case TextureType::Texture2DMSArray:
            DebugTextureSize(desc.texture2DMS.width);
            DebugTextureSize(desc.texture2DMS.height);
            if (desc.texture2DMS.layers == 0)
                ErrTextureLayersEqualZero();
            break;

        default:
            LLGL_DBG_ERROR(ErrorType::InvalidArgument, "invalid texture type");
            break;
    }
}

void DbgRenderSystem::DebugTextureSize(std::uint32_t size)
{
    if (size == 0)
        LLGL_DBG_ERROR(ErrorType::InvalidArgument, "invalid texture size");
}

void DbgRenderSystem::WarnTextureLayersGreaterOne()
{
    LLGL_DBG_WARN(WarningType::ImproperArgument, "texture layers is greater than 1 but no array texture is specified");
}

void DbgRenderSystem::ErrTextureLayersEqualZero()
{
    LLGL_DBG_ERROR(ErrorType::InvalidArgument, "number of texture layers must not be zero for array texutres");
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
