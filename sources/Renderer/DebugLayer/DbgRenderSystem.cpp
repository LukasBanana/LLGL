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

std::map<RendererInfo, std::string> DbgRenderSystem::QueryRendererInfo() const
{
    return instance_->QueryRendererInfo();
}

RenderingCaps DbgRenderSystem::QueryRenderingCaps() const
{
    return instance_->QueryRenderingCaps();
}

ShadingLanguage DbgRenderSystem::QueryShadingLanguage() const
{
    return instance_->QueryShadingLanguage();
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

    caps_ = instance_->QueryRenderingCaps();

    return TakeOwnership(renderContexts_, MakeUnique<DbgRenderContext>(
        *renderContextInstance, profiler_, debugger_, caps_
    ));
}

void DbgRenderSystem::Release(RenderContext& renderContext)
{
    instance_->Release(renderContext);
    RemoveFromUniqueSet(renderContexts_, &renderContext);
}

/* ----- Hardware Buffers ------ */

VertexBuffer* DbgRenderSystem::CreateVertexBuffer(std::size_t size, const BufferUsage usage, const VertexFormat& vertexFormat, const void* initialData)
{
    if (size % vertexFormat.GetFormatSize() != 0)
        LLGL_DBG_WARN_HERE(WarningType::ImproperArgument, "improper buffer size with vertex format of " + std::to_string(vertexFormat.GetFormatSize()) + " bytes");

    auto vertexBufferDbg = MakeUnique<DbgVertexBuffer>(*instance_->CreateVertexBuffer(size, usage, vertexFormat, initialData));

    vertexBufferDbg->format         = vertexFormat;
    vertexBufferDbg->size           = size;
    vertexBufferDbg->elements       = size / vertexFormat.GetFormatSize();
    vertexBufferDbg->initialized    = true;

    return TakeOwnership(vertexBuffers_, std::move(vertexBufferDbg));
}

IndexBuffer* DbgRenderSystem::CreateIndexBuffer(std::size_t size, const BufferUsage usage, const IndexFormat& indexFormat, const void* initialData)
{
    if (size % indexFormat.GetFormatSize() != 0)
        LLGL_DBG_WARN_HERE(WarningType::ImproperArgument, "improper buffer size with index format of " + std::to_string(indexFormat.GetFormatSize()) + " bytes");

    auto indexBufferDbg = MakeUnique<DbgIndexBuffer>(*instance_->CreateIndexBuffer(size, usage, indexFormat, initialData));

    indexBufferDbg->size        = size;
    indexBufferDbg->elements    = size / indexFormat.GetFormatSize();
    indexBufferDbg->initialized = true;

    return TakeOwnership(indexBuffers_, std::move(indexBufferDbg));
}

ConstantBuffer* DbgRenderSystem::CreateConstantBuffer(std::size_t size, const BufferUsage usage, const void* initialData)
{
    static const std::size_t packAlignment = 16;
    if (size % packAlignment != 0)
        LLGL_DBG_WARN_HERE(WarningType::ImproperArgument, "buffer size is out of pack alignment");

    auto constantBufferDbg = MakeUnique<DbgConstantBuffer>(*instance_->CreateConstantBuffer(size, usage, initialData));

    constantBufferDbg->size         = size;
    constantBufferDbg->initialized  = true;

    return TakeOwnership(constantBuffers_, std::move(constantBufferDbg));
}

StorageBuffer* DbgRenderSystem::CreateStorageBuffer(std::size_t size, const BufferUsage usage, const void* initialData)
{
    auto storageBufferDbg = MakeUnique<DbgStorageBuffer>(*instance_->CreateStorageBuffer(size, usage, initialData));

    storageBufferDbg->size          = size;
    storageBufferDbg->initialized   = true;

    return TakeOwnership(storageBuffers_, std::move(storageBufferDbg));
}

void DbgRenderSystem::Release(VertexBuffer& vertexBuffer)
{
    ReleaseDbg(vertexBuffers_, vertexBuffer);
}

void DbgRenderSystem::Release(IndexBuffer& indexBuffer)
{
    ReleaseDbg(indexBuffers_, indexBuffer);
}

void DbgRenderSystem::Release(ConstantBuffer& constantBuffer)
{
    ReleaseDbg(constantBuffers_, constantBuffer);
}

void DbgRenderSystem::Release(StorageBuffer& storageBuffer)
{
    ReleaseDbg(storageBuffers_, storageBuffer);
}

void DbgRenderSystem::WriteVertexBuffer(VertexBuffer& vertexBuffer, const void* data, std::size_t dataSize, std::size_t offset)
{
    auto& vertexBufferDbg = LLGL_CAST(DbgVertexBuffer&, vertexBuffer);
    if (vertexBufferDbg.initialized)
    {
        DebugBufferSize(vertexBufferDbg.size, dataSize, offset, __FUNCTION__);
        {
            instance_->WriteVertexBuffer(vertexBufferDbg.instance, data, dataSize, offset);
        }
        LLGL_DBG_PROFILER_DO(writeVertexBuffer.Inc());
    }
    else
        ErrWriteUninitializedResource(__FUNCTION__);
}

void DbgRenderSystem::WriteIndexBuffer(IndexBuffer& indexBuffer, const void* data, std::size_t dataSize, std::size_t offset)
{
    auto& indexBufferDbg = LLGL_CAST(DbgIndexBuffer&, indexBuffer);
    if (indexBufferDbg.initialized)
    {
        DebugBufferSize(indexBufferDbg.size, dataSize, offset, __FUNCTION__);
        {
            instance_->WriteIndexBuffer(indexBufferDbg.instance, data, dataSize, offset);
        }
        LLGL_DBG_PROFILER_DO(writeIndexBuffer.Inc());
    }
    else
        ErrWriteUninitializedResource(__FUNCTION__);
}

void DbgRenderSystem::WriteConstantBuffer(ConstantBuffer& constantBuffer, const void* data, std::size_t dataSize, std::size_t offset)
{
    auto& constantBufferDbg = LLGL_CAST(DbgConstantBuffer&, constantBuffer);
    if (constantBufferDbg.initialized)
    {
        DebugBufferSize(constantBufferDbg.size, dataSize, offset, __FUNCTION__);
        {
            instance_->WriteConstantBuffer(constantBufferDbg.instance, data, dataSize, offset);
        }
        LLGL_DBG_PROFILER_DO(writeConstantBuffer.Inc());
    }
    else
        ErrWriteUninitializedResource(__FUNCTION__);
}

void DbgRenderSystem::WriteStorageBuffer(StorageBuffer& storageBuffer, const void* data, std::size_t dataSize, std::size_t offset)
{
    auto& storageBufferDbg = LLGL_CAST(DbgStorageBuffer&, storageBuffer);
    if (storageBufferDbg.initialized)
    {
        DebugBufferSize(storageBufferDbg.size, dataSize, offset, __FUNCTION__);
        {
            instance_->WriteStorageBuffer(storageBufferDbg.instance, data, dataSize, offset);
        }
        LLGL_DBG_PROFILER_DO(writeStorageBuffer.Inc());
    }
    else
        ErrWriteUninitializedResource(__FUNCTION__);
}

/* ----- Textures ----- */

Texture* DbgRenderSystem::CreateTexture(const TextureDescriptor& textureDesc, const ImageDescriptor* imageDesc)
{
    DebugTextureDescriptor(textureDesc, __FUNCTION__);
    return TakeOwnership(textures_, MakeUnique<DbgTexture>(*instance_->CreateTexture(textureDesc, imageDesc), textureDesc));
}

void DbgRenderSystem::Release(Texture& texture)
{
    ReleaseDbg(textures_, texture);
}

TextureDescriptor DbgRenderSystem::QueryTextureDescriptor(const Texture& texture)
{
    auto& textureDbg = LLGL_CAST(const DbgTexture&, texture);
    return instance_->QueryTextureDescriptor(textureDbg.instance);
}

void DbgRenderSystem::WriteTexture(Texture& texture, const SubTextureDescriptor& subTextureDesc, const ImageDescriptor& imageDesc)
{
    auto& textureDbg = GetInitializedTexture(texture, __FUNCTION__);
    DebugMipLevelLimit(subTextureDesc.mipLevel, textureDbg.mipLevels, __FUNCTION__);
    {
        instance_->WriteTexture(textureDbg.instance, subTextureDesc, imageDesc);
    }
}

void DbgRenderSystem::ReadTexture(const Texture& texture, int mipLevel, ImageFormat dataFormat, DataType dataType, void* data)
{
    auto& textureDbg = LLGL_CAST(const DbgTexture&, texture);
    DebugMipLevelLimit(mipLevel, textureDbg.mipLevels, __FUNCTION__);
    {
        instance_->ReadTexture(textureDbg.instance, mipLevel, dataFormat, dataType, data);
    }
}

void DbgRenderSystem::GenerateMips(Texture& texture)
{
    auto& textureDbg = LLGL_CAST(DbgTexture&, texture);
    {
        instance_->GenerateMips(textureDbg.instance);
    }
    const auto& tex3DDesc = textureDbg.desc.texture3DDesc;
    textureDbg.mipLevels = NumMipLevels({ tex3DDesc.width, tex3DDesc.height, tex3DDesc.depth });
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
    if (desc.rasterizer.conservativeRasterization && !caps_.hasConservativeRasterization)
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

bool DbgRenderSystem::OnMakeCurrent(RenderContext* renderContext)
{
    return instance_->MakeCurrent(renderContext);
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

void DbgRenderSystem::ErrWriteUninitializedResource(const std::string& source)
{
    LLGL_DBG_ERROR(ErrorType::InvalidState, "attempt to write uninitialized resource", source);
}

void DbgRenderSystem::DebugTextureDescriptor(const TextureDescriptor& desc, const std::string& source)
{
    switch (desc.type)
    {
        case TextureType::Texture1D:
            DebugTextureSize(desc.texture1DDesc.width, source);
            if (desc.texture1DDesc.layers > 1)
                WarnTextureLayersGreaterOne(source);
            break;

        case TextureType::Texture2D:
        case TextureType::TextureCube:
            DebugTextureSize(desc.texture2DDesc.width, source);
            DebugTextureSize(desc.texture2DDesc.height, source);
            if (desc.texture2DDesc.layers > 1)
                WarnTextureLayersGreaterOne(source);
            break;

        case TextureType::Texture3D:
            DebugTextureSize(desc.texture3DDesc.width, source);
            DebugTextureSize(desc.texture3DDesc.height, source);
            DebugTextureSize(desc.texture3DDesc.depth, source);
            break;

        case TextureType::Texture1DArray:
            DebugTextureSize(desc.texture1DDesc.width, source);
            if (desc.texture1DDesc.layers == 0)
                ErrTextureLayersEqualZero(source);
            break;

        case TextureType::Texture2DArray:
        case TextureType::TextureCubeArray:
            DebugTextureSize(desc.texture2DDesc.width, source);
            DebugTextureSize(desc.texture2DDesc.height, source);
            if (desc.texture2DDesc.layers == 0)
                ErrTextureLayersEqualZero(source);
            break;

        default:
            LLGL_DBG_ERROR_HERE(ErrorType::InvalidArgument, "invalid texture type");
            break;
    }
}

void DbgRenderSystem::DebugTextureSize(int size, const std::string& source)
{
    if (size <= 0)
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

DbgTexture& DbgRenderSystem::GetInitializedTexture(Texture& texture, const std::string& source)
{
    auto& textureDbg = LLGL_CAST(DbgTexture&, texture);
    if (textureDbg.GetType() == TextureType::Undefined)
        ErrWriteUninitializedResource(source);
    return textureDbg;
}


} // /namespace LLGL



// ================================================================================
