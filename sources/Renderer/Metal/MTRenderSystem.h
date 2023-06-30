/*
 * MTRenderSystem.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_MT_RENDER_SYSTEM_H
#define LLGL_MT_RENDER_SYSTEM_H


#import <MetalKit/MetalKit.h>

#include <LLGL/RenderSystem.h>
#include "../ContainerTypes.h"

#include "Command/MTCommandQueue.h"
#include "Command/MTCommandBuffer.h"
#include "MTSwapChain.h"

#include "Buffer/MTBuffer.h"
#include "Buffer/MTBufferArray.h"

#include "RenderState/MTPipelineLayout.h"
#include "RenderState/MTPipelineState.h"
#include "RenderState/MTResourceHeap.h"
#include "RenderState/MTRenderPass.h"
#include "RenderState/MTFence.h"

#include "Shader/MTShader.h"

#include "Texture/MTTexture.h"
#include "Texture/MTSampler.h"
#include "Texture/MTRenderTarget.h"


namespace LLGL
{


class MTRenderSystem final : public RenderSystem
{

    public:

        /* ----- Common ----- */

        MTRenderSystem();
        ~MTRenderSystem();

        /* ----- Swap-chain ----- */

        SwapChain* CreateSwapChain(const SwapChainDescriptor& swapChainDesc, const std::shared_ptr<Surface>& surface = nullptr) override;

        void Release(SwapChain& swapChain) override;

        /* ----- Command queues ----- */

        CommandQueue* GetCommandQueue() override;

        /* ----- Command buffers ----- */

        CommandBuffer* CreateCommandBuffer(const CommandBufferDescriptor& commandBufferDesc = {}) override;

        void Release(CommandBuffer& commandBuffer) override;

        /* ----- Buffers ------ */

        Buffer* CreateBuffer(const BufferDescriptor& bufferDesc, const void* initialData = nullptr) override;
        BufferArray* CreateBufferArray(std::uint32_t numBuffers, Buffer* const * bufferArray) override;

        void Release(Buffer& buffer) override;
        void Release(BufferArray& bufferArray) override;

        void WriteBuffer(Buffer& buffer, std::uint64_t offset, const void* data, std::uint64_t dataSize) override;
        void ReadBuffer(Buffer& buffer, std::uint64_t offset, void* data, std::uint64_t dataSize) override;

        void* MapBuffer(Buffer& buffer, const CPUAccess access) override;
        void* MapBuffer(Buffer& buffer, const CPUAccess access, std::uint64_t offset, std::uint64_t length) override;
        void UnmapBuffer(Buffer& buffer) override;

        /* ----- Textures ----- */

        Texture* CreateTexture(const TextureDescriptor& textureDesc, const SrcImageDescriptor* imageDesc = nullptr) override;

        void Release(Texture& texture) override;

        void WriteTexture(Texture& texture, const TextureRegion& textureRegion, const SrcImageDescriptor& imageDesc) override;
        void ReadTexture(Texture& texture, const TextureRegion& textureRegion, const DstImageDescriptor& imageDesc) override;

        /* ----- Sampler States ---- */

        Sampler* CreateSampler(const SamplerDescriptor& samplerDesc) override;

        void Release(Sampler& sampler) override;

        /* ----- Resource Heaps ----- */

        ResourceHeap* CreateResourceHeap(const ResourceHeapDescriptor& resourceHeapDesc, const ArrayView<ResourceViewDescriptor>& initialResourceViews = {}) override;

        void Release(ResourceHeap& resourceHeap) override;

        std::uint32_t WriteResourceHeap(ResourceHeap& resourceHeap, std::uint32_t firstDescriptor, const ArrayView<ResourceViewDescriptor>& resourceViews) override;

        /* ----- Render Passes ----- */

        RenderPass* CreateRenderPass(const RenderPassDescriptor& renderPassDesc) override;

        void Release(RenderPass& renderPass) override;

        /* ----- Render Targets ----- */

        RenderTarget* CreateRenderTarget(const RenderTargetDescriptor& renderTargetDesc) override;

        void Release(RenderTarget& renderTarget) override;

        /* ----- Shader ----- */

        Shader* CreateShader(const ShaderDescriptor& shaderDesc) override;

        void Release(Shader& shader) override;

        /* ----- Pipeline Layouts ----- */

        PipelineLayout* CreatePipelineLayout(const PipelineLayoutDescriptor& pipelineLayoutDesc) override;

        void Release(PipelineLayout& pipelineLayout) override;

        /* ----- Pipeline States ----- */

        PipelineState* CreatePipelineState(const Blob& serializedCache) override;
        PipelineState* CreatePipelineState(const GraphicsPipelineDescriptor& pipelineStateDesc, Blob* serializedCache = nullptr) override;
        PipelineState* CreatePipelineState(const ComputePipelineDescriptor& pipelineStateDesc, Blob* serializedCache = nullptr) override;

        void Release(PipelineState& pipelineState) override;

        /* ----- Queries ----- */

        QueryHeap* CreateQueryHeap(const QueryHeapDescriptor& queryHeapDesc) override;

        void Release(QueryHeap& queryHeap) override;

        /* ----- Fences ----- */

        Fence* CreateFence() override;

        void Release(Fence& fence) override;

    private:

        void CreateDeviceResources();
        void QueryRenderingCaps();

        const char* QueryMetalVersion() const;

        MTLFeatureSet QueryHighestFeatureSet() const;

        const MTRenderPass* GetDefaultRenderPass() const;

    private:

        /* ----- Common objects ----- */

        id<MTLDevice>                       device_             = nil;

        /* ----- Hardware object containers ----- */

        HWObjectContainer<MTSwapChain>      swapChains_;
        HWObjectInstance<MTCommandQueue>    commandQueue_;
        HWObjectContainer<MTCommandBuffer>  commandBuffers_;
        HWObjectContainer<MTBuffer>         buffers_;
        HWObjectContainer<MTBufferArray>    bufferArrays_;
        HWObjectContainer<MTTexture>        textures_;
        HWObjectContainer<MTSampler>        samplers_;
        HWObjectContainer<MTRenderPass>     renderPasses_;
        HWObjectContainer<MTRenderTarget>   renderTargets_;
        HWObjectContainer<MTShader>         shaders_;
        HWObjectContainer<MTPipelineLayout> pipelineLayouts_;
        HWObjectContainer<MTPipelineState>  pipelineStates_;
        HWObjectContainer<MTResourceHeap>   resourceHeaps_;
        //HWObjectContainer<MTQueryHeap>      queryHeaps_;
        HWObjectContainer<MTFence>          fences_;

};


} // /namespace LLGL


#endif



// ================================================================================
