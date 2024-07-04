/*
 * NullRenderSystem.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "NullRenderSystem.h"
#include "../../Core/CoreUtils.h"
#include <LLGL/Utils/ForRange.h>
#include <limits.h>


namespace LLGL
{


static void InitNullRendererShadingLanguages(std::vector<ShadingLanguage>& shadingLanguages)
{
    shadingLanguages =
    {
        ShadingLanguage::GLSL,
        ShadingLanguage::ESSL,
        ShadingLanguage::SPIRV,
        ShadingLanguage::HLSL,
        ShadingLanguage::Metal,
    };
}

static void InitNullRendererTextureFormats(std::vector<Format>& textureFormats)
{
    constexpr int firstFormatIndex  = static_cast<int>(Format::A8UNorm);
    constexpr int lastFormatIndex   = static_cast<int>(Format::BC5SNorm);
    constexpr int numFormats        = lastFormatIndex - firstFormatIndex + 1;
    textureFormats.reserve(static_cast<std::size_t>(numFormats));
    for_range(i, numFormats)
        textureFormats.push_back(static_cast<Format>(firstFormatIndex + i));
}

static void InitNullRendererFeatures(RenderingFeatures& features)
{
    features.hasRenderTargets               = true;
    features.has3DTextures                  = true;
    features.hasCubeTextures                = true;
    features.hasArrayTextures               = true;
    features.hasCubeArrayTextures           = true;
    features.hasMultiSampleTextures         = true;
    features.hasTextureViews                = true;
    features.hasTextureViewSwizzle          = true;
    features.hasTextureViewFormatSwizzle    = true;
    features.hasBufferViews                 = true;
    features.hasConstantBuffers             = true;
    features.hasStorageBuffers              = true;
    features.hasGeometryShaders             = false;
    features.hasTessellationShaders         = false;
    features.hasTessellatorStage            = false;
    features.hasComputeShaders              = false;
    features.hasInstancing                  = true;
    features.hasOffsetInstancing            = true;
    features.hasIndirectDrawing             = true;
    features.hasViewportArrays              = true;
    features.hasConservativeRasterization   = false;
    features.hasStreamOutputs               = false;
    features.hasLogicOp                     = true;
    features.hasPipelineStatistics          = true;
    features.hasRenderCondition             = true;
}

static void InitNullRendererLimits(RenderingLimits& limits)
{
    limits.maxTextureArrayLayers            = 1024u;
    limits.maxColorAttachments              = LLGL_MAX_NUM_COLOR_ATTACHMENTS;
    limits.maxPatchVertices                 = 0;
    limits.max1DTextureSize                 = UINT32_MAX;
    limits.max2DTextureSize                 = UINT16_MAX;
    limits.max3DTextureSize                 = 1024u;
    limits.maxCubeTextureSize               = UINT16_MAX;
    limits.maxAnisotropy                    = 0;
    limits.maxComputeShaderWorkGroups[0]    = 0;
    limits.maxComputeShaderWorkGroups[1]    = 0;
    limits.maxComputeShaderWorkGroups[2]    = 0;
    limits.maxComputeShaderWorkGroupSize[0] = 0;
    limits.maxComputeShaderWorkGroupSize[1] = 0;
    limits.maxComputeShaderWorkGroupSize[2] = 0;
    limits.maxViewports                     = LLGL_MAX_NUM_VIEWPORTS_AND_SCISSORS;
    limits.maxViewportSize[0]               = UINT32_MAX;
    limits.maxViewportSize[1]               = UINT32_MAX;
    limits.maxBufferSize                    = SIZE_MAX;
    limits.maxConstantBufferSize            = UINT16_MAX;
    limits.maxStreamOutputs                 = LLGL_MAX_NUM_SO_BUFFERS;
    limits.maxTessFactor                    = 0;
    limits.minConstantBufferAlignment       = 1;
    limits.minSampledBufferAlignment        = 1;
    limits.minStorageBufferAlignment        = 1;
    limits.maxColorBufferSamples            = 1;
    limits.maxDepthBufferSamples            = 1;
    limits.maxStencilBufferSamples          = 1;
    limits.maxNoAttachmentSamples           = 1;
}

static void GetNullRenderingCaps(RenderingCapabilities& caps)
{
    InitNullRendererShadingLanguages(caps.shadingLanguages);
    InitNullRendererTextureFormats(caps.textureFormats);
    InitNullRendererFeatures(caps.features);
    InitNullRendererLimits(caps.limits);
}

static void GetNullRendererInfo(RendererInfo& info)
{
    info.rendererName           = "Null";
    info.deviceName             = "CPU";
    info.vendorName             = "LLGL";
    info.shadingLanguageName    = "Dummy";
}

NullRenderSystem::NullRenderSystem(const RenderSystemDescriptor& renderSystemDesc) :
    desc_         { renderSystemDesc               },
    commandQueue_ { MakeUnique<NullCommandQueue>() }
{
}

/* ----- Swap-chain ----- */

SwapChain* NullRenderSystem::CreateSwapChain(const SwapChainDescriptor& swapChainDesc, const std::shared_ptr<Surface>& surface)
{
    return swapChains_.emplace<NullSwapChain>(swapChainDesc, surface, GetRendererInfo());
}

void NullRenderSystem::Release(SwapChain& swapChain)
{
    swapChains_.erase(&swapChain);
}

/* ----- Command queues ----- */

CommandQueue* NullRenderSystem::GetCommandQueue()
{
    return commandQueue_.get();
}

/* ----- Command buffers ----- */

CommandBuffer* NullRenderSystem::CreateCommandBuffer(const CommandBufferDescriptor& commandBufferDesc)
{
    return commandBuffers_.emplace<NullCommandBuffer>(commandBufferDesc);
}

void NullRenderSystem::Release(CommandBuffer& commandBuffer)
{
    commandBuffers_.erase(&commandBuffer);
}

/* ----- Buffers ------ */

Buffer* NullRenderSystem::CreateBuffer(const BufferDescriptor& bufferDesc, const void* initialData)
{
    RenderSystem::AssertCreateBuffer(bufferDesc, GetRenderingCaps().limits.maxBufferSize);
    return buffers_.emplace<NullBuffer>(bufferDesc, initialData);
}

BufferArray* NullRenderSystem::CreateBufferArray(std::uint32_t numBuffers, Buffer* const * bufferArray)
{
    RenderSystem::AssertCreateBufferArray(numBuffers, bufferArray);
    return bufferArrays_.emplace<NullBufferArray>(numBuffers, bufferArray);
}

void NullRenderSystem::Release(Buffer& buffer)
{
    buffers_.erase(&buffer);
}

void NullRenderSystem::Release(BufferArray& bufferArray)
{
    bufferArrays_.erase(&bufferArray);
}

void NullRenderSystem::WriteBuffer(Buffer& buffer, std::uint64_t offset, const void* data, std::uint64_t dataSize)
{
    auto& bufferNull = LLGL_CAST(NullBuffer&, buffer);
    bufferNull.Write(offset, data, dataSize);
}

void NullRenderSystem::ReadBuffer(Buffer& buffer, std::uint64_t offset, void* data, std::uint64_t dataSize)
{
    auto& bufferNull = LLGL_CAST(NullBuffer&, buffer);
    bufferNull.Read(offset, data, dataSize);
}

void* NullRenderSystem::MapBuffer(Buffer& buffer, const CPUAccess access)
{
    auto& bufferNull = LLGL_CAST(NullBuffer&, buffer);
    return bufferNull.Map(access, 0, bufferNull.desc.size);
}

void* NullRenderSystem::MapBuffer(Buffer& buffer, const CPUAccess access, std::uint64_t offset, std::uint64_t length)
{
    auto& bufferNull = LLGL_CAST(NullBuffer&, buffer);
    return bufferNull.Map(access, offset, length);
}

void NullRenderSystem::UnmapBuffer(Buffer& buffer)
{
    auto& bufferNull = LLGL_CAST(NullBuffer&, buffer);
    bufferNull.Unmap();
}

/* ----- Textures ----- */

Texture* NullRenderSystem::CreateTexture(const TextureDescriptor& textureDesc, const ImageView* initialImage)
{
    return textures_.emplace<NullTexture>(textureDesc, initialImage);
}

void NullRenderSystem::Release(Texture& texture)
{
    textures_.erase(&texture);
}

void NullRenderSystem::WriteTexture(Texture& texture, const TextureRegion& textureRegion, const ImageView& srcImageDesc)
{
    auto& textureNull = LLGL_CAST(NullTexture&, texture);
    textureNull.Write(textureRegion, srcImageDesc);
}

void NullRenderSystem::ReadTexture(Texture& texture, const TextureRegion& textureRegion, const MutableImageView& dstImageView)
{
    auto& textureNull = LLGL_CAST(NullTexture&, texture);
    textureNull.Read(textureRegion, dstImageView);
}

/* ----- Sampler States ---- */

Sampler* NullRenderSystem::CreateSampler(const SamplerDescriptor& samplerDesc)
{
    return samplers_.emplace<NullSampler>(samplerDesc);
}

void NullRenderSystem::Release(Sampler& sampler)
{
    samplers_.erase(&sampler);
}

/* ----- Resource Views ----- */

ResourceHeap* NullRenderSystem::CreateResourceHeap(const ResourceHeapDescriptor& resourceHeapDesc, const ArrayView<ResourceViewDescriptor>& initialResourceViews)
{
    return resourceHeaps_.emplace<NullResourceHeap>(resourceHeapDesc, initialResourceViews);
}

void NullRenderSystem::Release(ResourceHeap& resourceHeap)
{
    resourceHeaps_.erase(&resourceHeap);
}

std::uint32_t NullRenderSystem::WriteResourceHeap(ResourceHeap& resourceHeap, std::uint32_t firstDescriptor, const ArrayView<ResourceViewDescriptor>& resourceViews)
{
    auto& resourceHeapNull = LLGL_CAST(NullResourceHeap&, resourceHeap);
    return resourceHeapNull.WriteResourceViews(firstDescriptor, resourceViews);
}

/* ----- Render Passes ----- */

RenderPass* NullRenderSystem::CreateRenderPass(const RenderPassDescriptor& renderPassDesc)
{
    return renderPasses_.emplace<NullRenderPass>(renderPassDesc);
}

void NullRenderSystem::Release(RenderPass& renderPass)
{
    renderPasses_.erase(&renderPass);
}

/* ----- Render Targets ----- */

RenderTarget* NullRenderSystem::CreateRenderTarget(const RenderTargetDescriptor& renderTargetDesc)
{
    return renderTargets_.emplace<NullRenderTarget>(renderTargetDesc);
}

void NullRenderSystem::Release(RenderTarget& renderTarget)
{
    renderTargets_.erase(&renderTarget);
}

/* ----- Shader ----- */

Shader* NullRenderSystem::CreateShader(const ShaderDescriptor& shaderDesc)
{
    return shaders_.emplace<NullShader>(shaderDesc);
}

void NullRenderSystem::Release(Shader& shader)
{
    shaders_.erase(&shader);
}

/* ----- Pipeline Layouts ----- */

PipelineLayout* NullRenderSystem::CreatePipelineLayout(const PipelineLayoutDescriptor& pipelineLayoutDesc)
{
    return pipelineLayouts_.emplace<NullPipelineLayout>(pipelineLayoutDesc);
}

void NullRenderSystem::Release(PipelineLayout& pipelineLayout)
{
    pipelineLayouts_.erase(&pipelineLayout);
}

/* ----- Pipeline Caches ----- */

PipelineCache* NullRenderSystem::CreatePipelineCache(const Blob& /*initialBlob*/)
{
    return ProxyPipelineCache::CreateInstance(pipelineCacheProxy_);
}

void NullRenderSystem::Release(PipelineCache& pipelineCache)
{
    ProxyPipelineCache::ReleaseInstance(pipelineCacheProxy_, pipelineCache);
}

/* ----- Pipeline States ----- */

PipelineState* NullRenderSystem::CreatePipelineState(const GraphicsPipelineDescriptor& pipelineStateDesc, PipelineCache* /*pipelineCache*/)
{
    return pipelineStates_.emplace<NullPipelineState>(pipelineStateDesc);
}

PipelineState* NullRenderSystem::CreatePipelineState(const ComputePipelineDescriptor& pipelineStateDesc, PipelineCache* /*pipelineCache*/)
{
    return pipelineStates_.emplace<NullPipelineState>(pipelineStateDesc);
}

void NullRenderSystem::Release(PipelineState& pipelineState)
{
    pipelineStates_.erase(&pipelineState);
}

/* ----- Queries ----- */

QueryHeap* NullRenderSystem::CreateQueryHeap(const QueryHeapDescriptor& queryHeapDesc)
{
    return queryHeaps_.emplace<NullQueryHeap>(queryHeapDesc);
}

void NullRenderSystem::Release(QueryHeap& queryHeap)
{
    queryHeaps_.erase(&queryHeap);
}

/* ----- Fences ----- */

Fence* NullRenderSystem::CreateFence()
{
    return fences_.emplace<NullFence>();
}

void NullRenderSystem::Release(Fence& fence)
{
    fences_.erase(&fence);
}

/* ----- Extensions ----- */

bool NullRenderSystem::GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize)
{
    return (nativeHandle == nullptr || nativeHandleSize == 0); // dummy
}


/*
 * ======= Private: =======
 */

bool NullRenderSystem::QueryRendererDetails(RendererInfo* outInfo, RenderingCapabilities* outCaps)
{
    if (outInfo != nullptr)
        GetNullRendererInfo(*outInfo);
    if (outCaps != nullptr)
        GetNullRenderingCaps(*outCaps);
    return true;
}


} // /namespace LLGL



// ================================================================================
