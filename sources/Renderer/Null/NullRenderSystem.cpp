/*
 * NullRenderSystem.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "NullRenderSystem.h"
#include "../../Core/Helper.h"
#include <LLGL/Misc/ForRange.h>
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
    features.hasDirectResourceBinding       = true;
    features.hasRenderTargets               = true;
    features.has3DTextures                  = true;
    features.hasCubeTextures                = true;
    features.hasArrayTextures               = true;
    features.hasCubeArrayTextures           = true;
    features.hasMultiSampleTextures         = true;
    features.hasTextureViews                = true;
    features.hasTextureViewSwizzle          = true;
    features.hasBufferViews                 = true;
    features.hasSamplers                    = true;
    features.hasConstantBuffers             = true;
    features.hasStorageBuffers              = true;
    features.hasUniforms                    = true;
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
}

static RenderingCapabilities GetNullRenderingCaps()
{
    RenderingCapabilities caps;
    InitNullRendererShadingLanguages(caps.shadingLanguages);
    InitNullRendererTextureFormats(caps.textureFormats);
    InitNullRendererFeatures(caps.features);
    InitNullRendererLimits(caps.limits);
    return caps;
}

static RendererInfo GetNullRenderInfo()
{
    RendererInfo info;
    info.rendererName           = "Null";
    info.deviceName             = "CPU";
    info.vendorName             = "LLGL";
    info.shadingLanguageName    = "Dummy";
    return info;
}

NullRenderSystem::NullRenderSystem(const RenderSystemDescriptor& desc) :
    desc_         { desc                           },
    commandQueue_ { MakeUnique<NullCommandQueue>() }
{
    SetRendererInfo(GetNullRenderInfo());
    SetRenderingCaps(GetNullRenderingCaps());
}

/* ----- Swap-chain ----- */

SwapChain* NullRenderSystem::CreateSwapChain(const SwapChainDescriptor& desc, const std::shared_ptr<Surface>& surface)
{
    return TakeOwnership(swapChains_, MakeUnique<NullSwapChain>(desc, surface));
}

void NullRenderSystem::Release(SwapChain& swapChain)
{
    RemoveFromUniqueSet(swapChains_, &swapChain);
}

/* ----- Command queues ----- */

CommandQueue* NullRenderSystem::GetCommandQueue()
{
    return commandQueue_.get();
}

/* ----- Command buffers ----- */

CommandBuffer* NullRenderSystem::CreateCommandBuffer(const CommandBufferDescriptor& desc)
{
    return TakeOwnership(commandBuffers_, MakeUnique<NullCommandBuffer>(desc));
}

void NullRenderSystem::Release(CommandBuffer& commandBuffer)
{
    RemoveFromUniqueSet(commandBuffers_, &commandBuffer);
}

/* ----- Buffers ------ */

Buffer* NullRenderSystem::CreateBuffer(const BufferDescriptor& desc, const void* initialData)
{
    if (desc.size > GetRenderingCaps().limits.maxBufferSize)
        throw std::invalid_argument("");
    return TakeOwnership(buffers_, MakeUnique<NullBuffer>(desc, initialData));
}

BufferArray* NullRenderSystem::CreateBufferArray(std::uint32_t numBuffers, Buffer* const * bufferArray)
{
    AssertCreateBufferArray(numBuffers, bufferArray);
    return TakeOwnership(bufferArrays_, MakeUnique<NullBufferArray>(numBuffers, bufferArray));
}

void NullRenderSystem::Release(Buffer& buffer)
{
    RemoveFromUniqueSet(buffers_, &buffer);
}

void NullRenderSystem::Release(BufferArray& bufferArray)
{
    RemoveFromUniqueSet(bufferArrays_, &bufferArray);
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

Texture* NullRenderSystem::CreateTexture(const TextureDescriptor& textureDesc, const SrcImageDescriptor* imageDesc)
{
    return TakeOwnership(textures_, MakeUnique<NullTexture>(textureDesc, imageDesc));
}

void NullRenderSystem::Release(Texture& texture)
{
    RemoveFromUniqueSet(textures_, &texture);
}

void NullRenderSystem::WriteTexture(Texture& texture, const TextureRegion& textureRegion, const SrcImageDescriptor& imageDesc)
{
    auto& textureNull = LLGL_CAST(NullTexture&, texture);
    textureNull.Write(textureRegion, imageDesc);
}

void NullRenderSystem::ReadTexture(Texture& texture, const TextureRegion& textureRegion, const DstImageDescriptor& imageDesc)
{
    auto& textureNull = LLGL_CAST(NullTexture&, texture);
    textureNull.Read(textureRegion, imageDesc);
}

/* ----- Sampler States ---- */

Sampler* NullRenderSystem::CreateSampler(const SamplerDescriptor& desc)
{
    return TakeOwnership(samplers_, MakeUnique<NullSampler>(desc));
}

void NullRenderSystem::Release(Sampler& sampler)
{
    RemoveFromUniqueSet(samplers_, &sampler);
}

/* ----- Resource Views ----- */

ResourceHeap* NullRenderSystem::CreateResourceHeap(const ResourceHeapDescriptor& desc)
{
    return TakeOwnership(resourceHeaps_, MakeUnique<NullResourceHeap>(desc));
}

void NullRenderSystem::Release(ResourceHeap& resourceHeap)
{
    RemoveFromUniqueSet(resourceHeaps_, &resourceHeap);
}

/* ----- Render Passes ----- */

RenderPass* NullRenderSystem::CreateRenderPass(const RenderPassDescriptor& desc)
{
    return TakeOwnership(renderPasses_, MakeUnique<NullRenderPass>(desc));
}

void NullRenderSystem::Release(RenderPass& renderPass)
{
    RemoveFromUniqueSet(renderPasses_, &renderPass);
}

/* ----- Render Targets ----- */

RenderTarget* NullRenderSystem::CreateRenderTarget(const RenderTargetDescriptor& desc)
{
    return TakeOwnership(renderTargets_, MakeUnique<NullRenderTarget>(desc));
}

void NullRenderSystem::Release(RenderTarget& renderTarget)
{
    RemoveFromUniqueSet(renderTargets_, &renderTarget);
}

/* ----- Shader ----- */

Shader* NullRenderSystem::CreateShader(const ShaderDescriptor& desc)
{
    return TakeOwnership(shaders_, MakeUnique<NullShader>(desc));
}

void NullRenderSystem::Release(Shader& shader)
{
    RemoveFromUniqueSet(shaders_, &shader);
}

/* ----- Pipeline Layouts ----- */

PipelineLayout* NullRenderSystem::CreatePipelineLayout(const PipelineLayoutDescriptor& desc)
{
    return TakeOwnership(pipelineLayouts_, MakeUnique<NullPipelineLayout>(desc));
}

void NullRenderSystem::Release(PipelineLayout& pipelineLayout)
{
    RemoveFromUniqueSet(pipelineLayouts_, &pipelineLayout);
}

/* ----- Pipeline States ----- */

PipelineState* NullRenderSystem::CreatePipelineState(const Blob& serializedCache)
{
    return nullptr;//TODO
}

PipelineState* NullRenderSystem::CreatePipelineState(const GraphicsPipelineDescriptor& desc, std::unique_ptr<Blob>* serializedCache)
{
    return TakeOwnership(pipelineStates_, MakeUnique<NullPipelineState>(desc));
}

PipelineState* NullRenderSystem::CreatePipelineState(const ComputePipelineDescriptor& desc, std::unique_ptr<Blob>* serializedCache)
{
    return TakeOwnership(pipelineStates_, MakeUnique<NullPipelineState>(desc));
}

void NullRenderSystem::Release(PipelineState& pipelineState)
{
    RemoveFromUniqueSet(pipelineStates_, &pipelineState);
}

/* ----- Queries ----- */

QueryHeap* NullRenderSystem::CreateQueryHeap(const QueryHeapDescriptor& desc)
{
    return TakeOwnership(queryHeaps_, MakeUnique<NullQueryHeap>(desc));
}

void NullRenderSystem::Release(QueryHeap& queryHeap)
{
    RemoveFromUniqueSet(queryHeaps_, &queryHeap);
}

/* ----- Fences ----- */

Fence* NullRenderSystem::CreateFence()
{
    return TakeOwnership(fences_, MakeUnique<NullFence>());
}

void NullRenderSystem::Release(Fence& fence)
{
    RemoveFromUniqueSet(fences_, &fence);
}


} // /namespace LLGL



// ================================================================================
