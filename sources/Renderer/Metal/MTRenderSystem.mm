/*
 * MTRenderSystem.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "MTRenderSystem.h"
#include "../CheckedCast.h"
#include "../../Core/Helper.h"


namespace LLGL
{


/* ----- Common ----- */

MTRenderSystem::MTRenderSystem()
{
    CreateDevice();
}

MTRenderSystem::~MTRenderSystem()
{
}

/* ----- Render Context ----- */

RenderContext* MTRenderSystem::CreateRenderContext(const RenderContextDescriptor& desc, const std::shared_ptr<Surface>& surface)
{
    return nullptr;//todo
}

void MTRenderSystem::Release(RenderContext& renderContext)
{
    //todo
    //RemoveFromUniqueSet(renderContexts_, &renderContext);
}

/* ----- Command queues ----- */

CommandQueue* MTRenderSystem::GetCommandQueue()
{
    return nullptr;//todo
    //return commandQueue_.get();
}

/* ----- Command buffers ----- */

CommandBuffer* MTRenderSystem::CreateCommandBuffer()
{
    return nullptr;//todo
}

CommandBufferExt* MTRenderSystem::CreateCommandBufferExt()
{
    return nullptr;//todo
}

void MTRenderSystem::Release(CommandBuffer& commandBuffer)
{
    //todo
    //RemoveFromUniqueSet(commandBuffers_, &commandBuffer);
}

/* ----- Buffers ------ */

Buffer* MTRenderSystem::CreateBuffer(const BufferDescriptor& desc, const void* initialData)
{
    return nullptr;//todo
}

BufferArray* MTRenderSystem::CreateBufferArray(std::uint32_t numBuffers, Buffer* const * bufferArray)
{
    return nullptr;//todo
}

void MTRenderSystem::Release(Buffer& buffer)
{
    //todo
}

void MTRenderSystem::Release(BufferArray& bufferArray)
{
    //todo
    //RemoveFromUniqueSet(bufferArrays_, &bufferArray);
}

void MTRenderSystem::WriteBuffer(Buffer& buffer, const void* data, std::size_t dataSize, std::size_t offset)
{
    //todo
}

void* MTRenderSystem::MapBuffer(Buffer& buffer, const CPUAccess access)
{
    return nullptr;//todo
}

void MTRenderSystem::UnmapBuffer(Buffer& buffer)
{
    //todo
}

/* ----- Textures ----- */

Texture* MTRenderSystem::CreateTexture(const TextureDescriptor& textureDesc, const SrcImageDescriptor* imageDesc)
{
    return nullptr;//todo
}

TextureArray* MTRenderSystem::CreateTextureArray(std::uint32_t numTextures, Texture* const * textureArray)
{
    return nullptr;//todo
}

void MTRenderSystem::Release(Texture& texture)
{
    //todo
}

void MTRenderSystem::Release(TextureArray& textureArray)
{
    //todo
}

void MTRenderSystem::WriteTexture(Texture& texture, const SubTextureDescriptor& subTextureDesc, const SrcImageDescriptor& imageDesc)
{
    //todo
}

void MTRenderSystem::ReadTexture(const Texture& texture, std::uint32_t mipLevel, const DstImageDescriptor& imageDesc)
{
    //todo
}

void MTRenderSystem::GenerateMips(Texture& texture)
{
    //todo
}

void MTRenderSystem::GenerateMips(Texture& texture, std::uint32_t baseMipLevel, std::uint32_t numMipLevels, std::uint32_t baseArrayLayer, std::uint32_t numArrayLayers)
{
    //todo
}

/* ----- Sampler States ---- */

Sampler* MTRenderSystem::CreateSampler(const SamplerDescriptor& desc)
{
    return nullptr;//todo
}

SamplerArray* MTRenderSystem::CreateSamplerArray(std::uint32_t numSamplers, Sampler* const * samplerArray)
{
    return nullptr;//todo
}

void MTRenderSystem::Release(Sampler& sampler)
{
    //todo
    //RemoveFromUniqueSet(samplers_, &sampler);
}

void MTRenderSystem::Release(SamplerArray& samplerArray)
{
    //todo
    //RemoveFromUniqueSet(samplerArrays_, &samplerArray);
}

/* ----- Resource Heaps ----- */

ResourceHeap* MTRenderSystem::CreateResourceHeap(const ResourceHeapDescriptor& desc)
{
    return nullptr;//todo
}

void MTRenderSystem::Release(ResourceHeap& resourceHeap)
{
    //todo
    //RemoveFromUniqueSet(resourceHeaps_, &resourceHeap);
}

/* ----- Render Targets ----- */

RenderTarget* MTRenderSystem::CreateRenderTarget(const RenderTargetDescriptor& desc)
{
    return nullptr;//todo
}

void MTRenderSystem::Release(RenderTarget& renderTarget)
{
    //todo
}

/* ----- Shader ----- */

Shader* MTRenderSystem::CreateShader(const ShaderType type)
{
    return nullptr;//todo
}

ShaderProgram* MTRenderSystem::CreateShaderProgram()
{
    return nullptr;//todo
}

void MTRenderSystem::Release(Shader& shader)
{
    //todo
    //RemoveFromUniqueSet(shaders_, &shader);
}

void MTRenderSystem::Release(ShaderProgram& shaderProgram)
{
    //todo
    //RemoveFromUniqueSet(shaderPrograms_, &shaderProgram);
}

/* ----- Pipeline Layouts ----- */

PipelineLayout* MTRenderSystem::CreatePipelineLayout(const PipelineLayoutDescriptor& desc)
{
    return nullptr;//todo
}

void MTRenderSystem::Release(PipelineLayout& pipelineLayout)
{
    //todo
    //RemoveFromUniqueSet(pipelineLayouts_, &pipelineLayout);
}

/* ----- Pipeline States ----- */

GraphicsPipeline* MTRenderSystem::CreateGraphicsPipeline(const GraphicsPipelineDescriptor& desc)
{
    return nullptr;//todo
}

ComputePipeline* MTRenderSystem::CreateComputePipeline(const ComputePipelineDescriptor& desc)
{
    return nullptr;//todo
}

void MTRenderSystem::Release(GraphicsPipeline& graphicsPipeline)
{
    //todo
    //RemoveFromUniqueSet(graphicsPipelines_, &graphicsPipeline);
}

void MTRenderSystem::Release(ComputePipeline& computePipeline)
{
    //todo
    //RemoveFromUniqueSet(computePipelines_, &computePipeline);
}

/* ----- Queries ----- */

Query* MTRenderSystem::CreateQuery(const QueryDescriptor& desc)
{
    return nullptr;//todo
}

void MTRenderSystem::Release(Query& query)
{
    //todo
    //RemoveFromUniqueSet(queries_, &query);
}

/* ----- Fences ----- */

Fence* MTRenderSystem::CreateFence()
{
    return nullptr;//todo
}

void MTRenderSystem::Release(Fence& fence)
{
    //todo
    //RemoveFromUniqueSet(fences_, &fence);
}


/*
 * ======= Private: =======
 */

void MTRenderSystem::CreateDevice()
{
    device_ = MTLCreateSystemDefaultDevice();
    if (device_ == nil)
        throw std::runtime_error("failed to create Metal device");
}


} // /namespace LLGL



// ================================================================================
