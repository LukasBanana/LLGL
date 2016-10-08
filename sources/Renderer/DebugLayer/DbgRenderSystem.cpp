/*
 * DbgRenderSystem.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
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
        instance_( instance ),
        profiler_( profiler ),
        debugger_( debugger )
{
    DetermineRenderer(instance_->GetName());
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

RenderContext* DbgRenderSystem::CreateRenderContext(const RenderContextDescriptor& desc, const std::shared_ptr<Window>& window)
{
    auto renderContextInstance = instance_->CreateRenderContext(desc, window);

    SetRendererInfo(instance_->GetRendererInfo());
    SetRenderingCaps(instance_->GetRenderingCaps());

    return TakeOwnership(renderContexts_, MakeUnique<DbgRenderContext>(*renderContextInstance));
}

void DbgRenderSystem::Release(RenderContext& renderContext)
{
    ReleaseDbg(renderContexts_, renderContext);
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
    unsigned int formatSize = 0;

    switch (desc.type)
    {
        case BufferType::Vertex:
        {
            /* Validate buffer size for specified vertex format */
            formatSize = desc.vertexBuffer.format.stride;
            if (desc.size % formatSize != 0)
                LLGL_DBG_WARN_HERE(WarningType::ImproperArgument, "improper vertex buffer size with vertex format of " + std::to_string(formatSize) + " bytes");
        }
        break;

        case BufferType::Index:
        {
            /* Validate buffer size for specified index format */
            formatSize = desc.indexBuffer.format.GetFormatSize();
            if (desc.size % formatSize != 0)
                LLGL_DBG_WARN_HERE(WarningType::ImproperArgument, "improper index buffer size with index format of " + std::to_string(formatSize) + " bytes");
        }
        break;

        case BufferType::Constant:
        {
            /* Validate pack alginemnt of 16 bytes */
            static const std::size_t packAlignment = 16;
            if (desc.size % packAlignment != 0)
                LLGL_DBG_WARN_HERE(WarningType::ImproperArgument, "constant buffer size is out of pack alignment (alignment is 16 bytes)");
        }
        break;
        
        default:
        break;
    }

    /* Create buffer object */
    auto bufferDbg = MakeUnique<DbgBuffer>(*instance_->CreateBuffer(desc, initialData), desc.type);

    /* Store settings */
    bufferDbg->desc         = desc;
    bufferDbg->elements     = (formatSize > 0 ? desc.size / formatSize : 0);
    bufferDbg->initialized  = (initialData != nullptr);

    return TakeOwnership(buffers_, std::move(bufferDbg));
}

BufferArray* DbgRenderSystem::CreateBufferArray(unsigned int numBuffers, Buffer* const * bufferArray)
{
    AssertCreateBufferArray(numBuffers, bufferArray);

    /* Create temporary buffer array with buffer instances */
    std::vector<Buffer*> bufferInstanceArray;
    for (unsigned int i = 0; i < numBuffers; ++i)
    {
        auto bufferDbg = LLGL_CAST(DbgBuffer*, (*(bufferArray++)));
        bufferInstanceArray.push_back(&(bufferDbg->instance));
    }

    return instance_->CreateBufferArray(numBuffers, bufferInstanceArray.data());
}

void DbgRenderSystem::Release(Buffer& buffer)
{
    ReleaseDbg(buffers_, buffer);
}

void DbgRenderSystem::Release(BufferArray& bufferArray)
{
    instance_->Release(bufferArray);
    //ReleaseDbg(bufferArrays_, bufferArray);
}

void DbgRenderSystem::WriteBuffer(Buffer& buffer, const void* data, std::size_t dataSize, std::size_t offset)
{
    auto& bufferDbg = LLGL_CAST(DbgBuffer&, buffer);
    
    /* Make a rough approximation if the buffer is now being initialized */
    if (!bufferDbg.initialized)
    {
        if (offset == 0)
            bufferDbg.initialized = true;
    }

    DebugBufferSize(bufferDbg.desc.size, dataSize, offset, __FUNCTION__);
    {
        instance_->WriteBuffer(bufferDbg.instance, data, dataSize, offset);
    }
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
    DebugTextureDescriptor(textureDesc, __FUNCTION__);
    return TakeOwnership(textures_, MakeUnique<DbgTexture>(*instance_->CreateTexture(textureDesc, imageDesc), textureDesc));
}

TextureArray* DbgRenderSystem::CreateTextureArray(unsigned int numTextures, Texture* const * textureArray)
{
    AssertCreateTextureArray(numTextures, textureArray);

    /* Create temporary buffer array with buffer instances */
    std::vector<Texture*> textureInstanceArray;
    for (unsigned int i = 0; i < numTextures; ++i)
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

TextureDescriptor DbgRenderSystem::QueryTextureDescriptor(const Texture& texture)
{
    auto& textureDbg = LLGL_CAST(const DbgTexture&, texture);
    return instance_->QueryTextureDescriptor(textureDbg.instance);
}

void DbgRenderSystem::WriteTexture(Texture& texture, const SubTextureDescriptor& subTextureDesc, const ImageDescriptor& imageDesc)
{
    auto& textureDbg = LLGL_CAST(DbgTexture&, texture);
    DebugMipLevelLimit(subTextureDesc.mipLevel, textureDbg.mipLevels, __FUNCTION__);
    {
        instance_->WriteTexture(textureDbg.instance, subTextureDesc, imageDesc);
    }
}

void DbgRenderSystem::ReadTexture(const Texture& texture, int mipLevel, ImageFormat imageFormat, DataType dataType, void* buffer)
{
    auto& textureDbg = LLGL_CAST(const DbgTexture&, texture);
    DebugMipLevelLimit(mipLevel, textureDbg.mipLevels, __FUNCTION__);
    {
        instance_->ReadTexture(textureDbg.instance, mipLevel, imageFormat, dataType, buffer);
    }
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

void DbgRenderSystem::Release(Sampler& sampler)
{
    instance_->Release(sampler);
    //RemoveFromUniqueSet(samplers_, &sampler);
}

/* ----- Render Targets ----- */

RenderTarget* DbgRenderSystem::CreateRenderTarget(unsigned int multiSamples)
{
    return TakeOwnership(renderTargets_, MakeUnique<DbgRenderTarget>(*instance_->CreateRenderTarget(multiSamples), multiSamples));
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
    if (desc.rasterizer.conservativeRasterization && !GetRenderingCaps().hasConservativeRasterization)
        LLGL_DBG_ERROR_NOT_SUPPORTED("conservative rasterization", __FUNCTION__);
    if (desc.blend.targets.size() > 8)
        LLGL_DBG_ERROR_HERE(ErrorType::InvalidArgument, "too many blend state targets (limit is 8)");

    if (renderer_.isDirect3D)
    {
        switch (desc.primitiveTopology)
        {
            case PrimitiveTopology::LineLoop:
                LLGL_DBG_ERROR(ErrorType::InvalidArgument, "renderer does not support primitive topology line loop", __FUNCTION__);
                break;
            case PrimitiveTopology::TriangleFan:
                LLGL_DBG_ERROR(ErrorType::InvalidArgument, "renderer does not support primitive topology triangle fan", __FUNCTION__);
                break;
            default:
                break;
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
    {
        LLGL_DBG_ERROR_HERE(ErrorType::InvalidArgument, "shader program must not be null");
        return nullptr;
    }
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
    {
        LLGL_DBG_ERROR_HERE(ErrorType::InvalidArgument, "shader program must not be null");
        return nullptr;
    }
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


/*
 * ======= Private: =======
 */

//TODO: replace this by 'RendererInfo::rendererID'
void DbgRenderSystem::DetermineRenderer(const std::string& rendererName)
{
    auto CompareSubStr = [](const std::string& lhs, const std::string& rhs)
    {
        return (lhs.size() >= rhs.size() && lhs.substr(0, rhs.size()) == rhs);
    };

    /* Determine renderer API by specified name */
    if (CompareSubStr(rendererName, "OpenGL"))
        renderer_.isOpenGL = true;
    else if (CompareSubStr(rendererName, "Direct3D"))
        renderer_.isDirect3D = true;
    else if (CompareSubStr(rendererName, "Vulkan"))
        renderer_.isVulkan = true;
}

void DbgRenderSystem::DebugBufferSize(std::size_t bufferSize, std::size_t dataSize, std::size_t dataOffset, const std::string& source)
{
    if (dataSize + dataOffset > bufferSize)
        LLGL_DBG_ERROR(ErrorType::InvalidArgument, "buffer size and offset out of bounds", source);
}

void DbgRenderSystem::DebugMipLevelLimit(int mipLevel, int mipLevelCount, const std::string& source)
{
    if (mipLevel >= mipLevelCount)
    {
        LLGL_DBG_ERROR(
            ErrorType::InvalidArgument,
            "mip level out of bounds (" + std::to_string(mipLevel) +
            " specified but limit is " + std::to_string(mipLevelCount - 1) + ")",
            source
        );
    }
}

void DbgRenderSystem::DebugTextureDescriptor(const TextureDescriptor& desc, const std::string& source)
{
    switch (desc.type)
    {
        case TextureType::Texture1D:
            DebugTextureSize(desc.texture1D.width, source);
            if (desc.texture1D.layers > 1)
                WarnTextureLayersGreaterOne(source);
            break;

        case TextureType::Texture2D:
        case TextureType::TextureCube:
            DebugTextureSize(desc.texture2D.width, source);
            DebugTextureSize(desc.texture2D.height, source);
            if (desc.texture2D.layers > 1)
                WarnTextureLayersGreaterOne(source);
            break;

        case TextureType::Texture3D:
            DebugTextureSize(desc.texture3D.width, source);
            DebugTextureSize(desc.texture3D.height, source);
            DebugTextureSize(desc.texture3D.depth, source);
            break;

        case TextureType::Texture1DArray:
            DebugTextureSize(desc.texture1D.width, source);
            if (desc.texture1D.layers == 0)
                ErrTextureLayersEqualZero(source);
            break;

        case TextureType::Texture2DArray:
        case TextureType::TextureCubeArray:
            DebugTextureSize(desc.texture2D.width, source);
            DebugTextureSize(desc.texture2D.height, source);
            if (desc.texture2D.layers == 0)
                ErrTextureLayersEqualZero(source);
            break;

        case TextureType::Texture2DMS:
            DebugTextureSize(desc.texture2DMS.width, source);
            DebugTextureSize(desc.texture2DMS.height, source);
            if (desc.texture2DMS.layers > 1)
                WarnTextureLayersGreaterOne(source);
            break;

        case TextureType::Texture2DMSArray:
            DebugTextureSize(desc.texture2DMS.width, source);
            DebugTextureSize(desc.texture2DMS.height, source);
            if (desc.texture2DMS.layers == 0)
                ErrTextureLayersEqualZero(source);
            break;

        default:
            LLGL_DBG_ERROR_HERE(ErrorType::InvalidArgument, "invalid texture type");
            break;
    }
}

void DbgRenderSystem::DebugTextureSize(unsigned int size, const std::string& source)
{
    if (size == 0)
        LLGL_DBG_ERROR(ErrorType::InvalidArgument, "invalid texture size", source);
}

void DbgRenderSystem::WarnTextureLayersGreaterOne(const std::string& source)
{
    LLGL_DBG_WARN(WarningType::ImproperArgument, "texture layers is greater than 1 but no array texture is specified", source);
}

void DbgRenderSystem::ErrTextureLayersEqualZero(const std::string& source)
{
    LLGL_DBG_ERROR(ErrorType::InvalidArgument, "number of texture layers must not be zero for array texutres", source);
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
