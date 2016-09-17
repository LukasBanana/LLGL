/*
 * DbgRenderSystem.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "DbgRenderSystem.h"
#include "../../Core/Helper.h"


namespace LLGL
{


DbgRenderSystem::DbgRenderSystem(const std::shared_ptr<RenderSystem>& instance, RenderingProfiler& profiler) :
    instance_( instance ),
    profiler_( profiler )
{
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

/* ----- Render Context ----- */

RenderContext* DbgRenderSystem::CreateRenderContext(const RenderContextDescriptor& desc, const std::shared_ptr<Window>& window)
{
    return TakeOwnership(renderContexts_, MakeUnique<DbgRenderContext>(
        *instance_->CreateRenderContext(desc, window), profiler_
    ));
}

void DbgRenderSystem::Release(RenderContext& renderContext)
{
    instance_->Release(renderContext);
    RemoveFromUniqueSet(renderContexts_, &renderContext);
}

/* ----- Hardware Buffers ------ */

VertexBuffer* DbgRenderSystem::CreateVertexBuffer()
{
    return instance_->CreateVertexBuffer();
    //return TakeOwnership(vertexBuffers_, MakeUnique<DbgVertexBuffer>());
}

IndexBuffer* DbgRenderSystem::CreateIndexBuffer()
{
    return instance_->CreateIndexBuffer();
    //return TakeOwnership(indexBuffers_, MakeUnique<DbgIndexBuffer>());
}

ConstantBuffer* DbgRenderSystem::CreateConstantBuffer()
{
    return instance_->CreateConstantBuffer();
    //return TakeOwnership(constantBuffers_, MakeUnique<DbgConstantBuffer>(device_.Get()));
}

StorageBuffer* DbgRenderSystem::CreateStorageBuffer()
{
    return instance_->CreateStorageBuffer();
    //return TakeOwnership(storageBuffers_, MakeUnique<DbgStorageBuffer>());
}

void DbgRenderSystem::Release(VertexBuffer& vertexBuffer)
{
    instance_->Release(vertexBuffer);
    //RemoveFromUniqueSet(vertexBuffers_, &vertexBuffer);
}

void DbgRenderSystem::Release(IndexBuffer& indexBuffer)
{
    instance_->Release(indexBuffer);
    //RemoveFromUniqueSet(indexBuffers_, &indexBuffer);
}

void DbgRenderSystem::Release(ConstantBuffer& constantBuffer)
{
    instance_->Release(constantBuffer);
    //RemoveFromUniqueSet(constantBuffers_, &constantBuffer);
}

void DbgRenderSystem::Release(StorageBuffer& storageBuffer)
{
    instance_->Release(storageBuffer);
    //RemoveFromUniqueSet(storageBuffers_, &storageBuffer);
}

void DbgRenderSystem::SetupVertexBuffer(
    VertexBuffer& vertexBuffer, const void* data, std::size_t dataSize, const BufferUsage usage, const VertexFormat& vertexFormat)
{
    instance_->SetupVertexBuffer(vertexBuffer, data, dataSize, usage, vertexFormat);
}

void DbgRenderSystem::SetupIndexBuffer(
    IndexBuffer& indexBuffer, const void* data, std::size_t dataSize, const BufferUsage usage, const IndexFormat& indexFormat)
{
    instance_->SetupIndexBuffer(indexBuffer, data, dataSize, usage, indexFormat);
}

void DbgRenderSystem::SetupConstantBuffer(
    ConstantBuffer& constantBuffer, const void* data, std::size_t dataSize, const BufferUsage usage)
{
    instance_->SetupConstantBuffer(constantBuffer, data, dataSize, usage);
}

void DbgRenderSystem::SetupStorageBuffer(
    StorageBuffer& storageBuffer, const void* data, std::size_t dataSize, const BufferUsage usage)
{
    instance_->SetupStorageBuffer(storageBuffer, data, dataSize, usage);
}

void DbgRenderSystem::WriteVertexBuffer(VertexBuffer& vertexBuffer, const void* data, std::size_t dataSize, std::size_t offset)
{
    instance_->WriteVertexBuffer(vertexBuffer, data, dataSize, offset);
    profiler_.writeVertexBuffer.Inc();
}

void DbgRenderSystem::WriteIndexBuffer(IndexBuffer& indexBuffer, const void* data, std::size_t dataSize, std::size_t offset)
{
    instance_->WriteIndexBuffer(indexBuffer, data, dataSize, offset);
    profiler_.writeIndexBuffer.Inc();
}

void DbgRenderSystem::WriteConstantBuffer(ConstantBuffer& constantBuffer, const void* data, std::size_t dataSize, std::size_t offset)
{
    instance_->WriteConstantBuffer(constantBuffer, data, dataSize, offset);
    profiler_.writeConstantBuffer.Inc();
}

void DbgRenderSystem::WriteStorageBuffer(StorageBuffer& storageBuffer, const void* data, std::size_t dataSize, std::size_t offset)
{
    instance_->WriteStorageBuffer(storageBuffer, data, dataSize, offset);
    profiler_.writeStorageBuffer.Inc();
}

/* ----- Textures ----- */

Texture* DbgRenderSystem::CreateTexture()
{
    return instance_->CreateTexture();
    //return TakeOwnership(textures_, MakeUnique<DbgTexture>());
}

void DbgRenderSystem::Release(Texture& texture)
{
    instance_->Release(texture);
    //RemoveFromUniqueSet(textures_, &texture);
}

TextureDescriptor DbgRenderSystem::QueryTextureDescriptor(const Texture& texture)
{
    return instance_->QueryTextureDescriptor(texture);
}

void DbgRenderSystem::SetupTexture1D(Texture& texture, const TextureFormat format, int size, const ImageDataDescriptor* imageDesc)
{
    instance_->SetupTexture1D(texture, format, size, imageDesc);
}

void DbgRenderSystem::SetupTexture2D(Texture& texture, const TextureFormat format, const Gs::Vector2i& size, const ImageDataDescriptor* imageDesc)
{
    instance_->SetupTexture2D(texture, format, size, imageDesc);
}

void DbgRenderSystem::SetupTexture3D(Texture& texture, const TextureFormat format, const Gs::Vector3i& size, const ImageDataDescriptor* imageDesc)
{
    instance_->SetupTexture3D(texture, format, size, imageDesc);
}

void DbgRenderSystem::SetupTextureCube(Texture& texture, const TextureFormat format, const Gs::Vector2i& size, const ImageDataDescriptor* imageDesc)
{
    instance_->SetupTextureCube(texture, format, size, imageDesc);
}

void DbgRenderSystem::SetupTexture1DArray(Texture& texture, const TextureFormat format, int size, unsigned int layers, const ImageDataDescriptor* imageDesc)
{
    instance_->SetupTexture1DArray(texture, format, size, layers, imageDesc);
}

void DbgRenderSystem::SetupTexture2DArray(Texture& texture, const TextureFormat format, const Gs::Vector2i& size, unsigned int layers, const ImageDataDescriptor* imageDesc)
{
    instance_->SetupTexture2DArray(texture, format, size, layers, imageDesc);
}

void DbgRenderSystem::SetupTextureCubeArray(Texture& texture, const TextureFormat format, const Gs::Vector2i& size, unsigned int layers, const ImageDataDescriptor* imageDesc)
{
    instance_->SetupTextureCubeArray(texture, format, size, layers, imageDesc);
}

void DbgRenderSystem::WriteTexture1D(
    Texture& texture, int mipLevel, int position, int size, const ImageDataDescriptor& imageDesc)
{
    instance_->WriteTexture1D(texture, mipLevel, position, size, imageDesc);
}

void DbgRenderSystem::WriteTexture2D(
    Texture& texture, int mipLevel, const Gs::Vector2i& position, const Gs::Vector2i& size, const ImageDataDescriptor& imageDesc)
{
    instance_->WriteTexture2D(texture, mipLevel, position, size, imageDesc);
}

void DbgRenderSystem::WriteTexture3D(
    Texture& texture, int mipLevel, const Gs::Vector3i& position, const Gs::Vector3i& size, const ImageDataDescriptor& imageDesc)
{
    instance_->WriteTexture3D(texture, mipLevel, position, size, imageDesc);
}

void DbgRenderSystem::WriteTextureCube(
    Texture& texture, int mipLevel, const Gs::Vector2i& position, const AxisDirection cubeFace, const Gs::Vector2i& size, const ImageDataDescriptor& imageDesc)
{
    instance_->WriteTextureCube(texture, mipLevel, position, cubeFace, size, imageDesc);
}

void DbgRenderSystem::WriteTexture1DArray(
    Texture& texture, int mipLevel, int position, unsigned int layerOffset,
    int size, unsigned int layers, const ImageDataDescriptor& imageDesc)
{
    instance_->WriteTexture1DArray(texture, mipLevel, position, layerOffset, size, layers, imageDesc);
}

void DbgRenderSystem::WriteTexture2DArray(
    Texture& texture, int mipLevel, const Gs::Vector2i& position, unsigned int layerOffset,
    const Gs::Vector2i& size, unsigned int layers, const ImageDataDescriptor& imageDesc)
{
    instance_->WriteTexture2DArray(texture, mipLevel, position, layerOffset, size, layers, imageDesc);
}

void DbgRenderSystem::WriteTextureCubeArray(
    Texture& texture, int mipLevel, const Gs::Vector2i& position, unsigned int layerOffset, const AxisDirection cubeFaceOffset,
    const Gs::Vector2i& size, unsigned int cubeFaces, const ImageDataDescriptor& imageDesc)
{
    instance_->WriteTextureCubeArray(texture, mipLevel, position, layerOffset, cubeFaceOffset, size, cubeFaces, imageDesc);
}

void DbgRenderSystem::ReadTexture(const Texture& texture, int mipLevel, ColorFormat dataFormat, DataType dataType, void* data)
{
    instance_->ReadTexture(texture, mipLevel, dataFormat, dataType, data);
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
    return instance_->CreateRenderTarget(multiSamples);
    //TakeOwnership(renderTargets_, MakeUnique<DbgRenderTarget>(multiSamples));
}

void DbgRenderSystem::Release(RenderTarget& renderTarget)
{
    instance_->Release(renderTarget);
    //RemoveFromUniqueSet(renderTargets_, &renderTarget);
}

/* ----- Shader ----- */

Shader* DbgRenderSystem::CreateShader(const ShaderType type)
{
    return instance_->CreateShader(type);
    //return TakeOwnership(shaders_, MakeUnique<DbgShader>(type));
}

ShaderProgram* DbgRenderSystem::CreateShaderProgram()
{
    return instance_->CreateShaderProgram();
    //return TakeOwnership(shaderPrograms_, MakeUnique<DbgShaderProgram>());
}

void DbgRenderSystem::Release(Shader& shader)
{
    instance_->Release(shader);
    //RemoveFromUniqueSet(shaders_, &shader);
}

void DbgRenderSystem::Release(ShaderProgram& shaderProgram)
{
    instance_->Release(shaderProgram);
    //RemoveFromUniqueSet(shaderPrograms_, &shaderProgram);
}

/* ----- Pipeline States ----- */

GraphicsPipeline* DbgRenderSystem::CreateGraphicsPipeline(const GraphicsPipelineDescriptor& desc)
{
    return instance_->CreateGraphicsPipeline(desc);
    //return TakeOwnership(graphicsPipelines_, MakeUnique<DbgGraphicsPipeline>(*this, desc));
}

ComputePipeline* DbgRenderSystem::CreateComputePipeline(const ComputePipelineDescriptor& desc)
{
    return instance_->CreateComputePipeline(desc);
    //return TakeOwnership(computePipelines_, MakeUnique<DbgComputePipeline>(*this, desc));
}

void DbgRenderSystem::Release(GraphicsPipeline& graphicsPipeline)
{
    instance_->Release(graphicsPipeline);
    //RemoveFromUniqueSet(graphicsPipelines_, &graphicsPipeline);
}

void DbgRenderSystem::Release(ComputePipeline& computePipeline)
{
    instance_->Release(computePipeline);
    //RemoveFromUniqueSet(computePipelines_, &computePipeline);
}

/* ----- Queries ----- */

Query* DbgRenderSystem::CreateQuery(const QueryType type)
{
    return instance_->CreateQuery(type);
    //return TakeOwnership(queries_, MakeUnique<DbgQuery>(*this, type));
}

void DbgRenderSystem::Release(Query& query)
{
    instance_->Release(query);
    //RemoveFromUniqueSet(queries_, &query);
}


/*
 * ======= Private: =======
 */

bool DbgRenderSystem::OnMakeCurrent(RenderContext* renderContext)
{
    return instance_->MakeCurrent(renderContext);
}



} // /namespace LLGL



// ================================================================================
