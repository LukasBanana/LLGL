/*
 * NullRenderSystem.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_NULL_RENDER_SYSTEM_H
#define LLGL_NULL_RENDER_SYSTEM_H


#include <LLGL/RenderSystem.h>
#include "NullSwapChain.h"
#include "Command/NullCommandBuffer.h"
#include "Command/NullCommandQueue.h"
#include "Buffer/NullBuffer.h"
#include "Buffer/NullBufferArray.h"
#include "RenderState/NullFence.h"
#include "RenderState/NullPipelineLayout.h"
#include "RenderState/NullPipelineState.h"
#include "RenderState/NullQueryHeap.h"
#include "RenderState/NullResourceHeap.h"
#include "RenderState/NullRenderPass.h"
#include "Shader/NullShader.h"
#include "Texture/NullTexture.h"
#include "Texture/NullRenderTarget.h"
#include "Texture/NullSampler.h"

#include "../ContainerTypes.h"


namespace LLGL
{


class NullRenderSystem final : public RenderSystem
{

    public:

        NullRenderSystem(const RenderSystemDescriptor& renderSystemDesc);

        /* ----- Swap-chain ------ */

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

        /* ----- Resource Views ----- */

        ResourceHeap* CreateResourceHeap(const ResourceHeapDescriptor& resourceHeapDesc, const ArrayView<ResourceViewDescriptor>& initialResourceViews = {}) override;

        void Release(ResourceHeap& resourceHeap) override;

        void WriteResourceHeap(ResourceHeap& resourceHeap, std::uint32_t firstDescriptor, const ArrayView<ResourceViewDescriptor>& resourceViews) override;

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
        PipelineState* CreatePipelineState(const GraphicsPipelineDescriptor& pipelineStateDesc, std::unique_ptr<Blob>* serializedCache = nullptr) override;
        PipelineState* CreatePipelineState(const ComputePipelineDescriptor& pipelineStateDesc, std::unique_ptr<Blob>* serializedCache = nullptr) override;

        void Release(PipelineState& pipelineState) override;

        /* ----- Queries ----- */

        QueryHeap* CreateQueryHeap(const QueryHeapDescriptor& queryHeapDesc) override;

        void Release(QueryHeap& queryHeap) override;

        /* ----- Fences ----- */

        Fence* CreateFence() override;

        void Release(Fence& fence) override;

    private:

        /* ----- Common objects ----- */

        const RenderSystemDescriptor            desc_;

        /* ----- Hardware object containers ----- */

        HWObjectContainer<NullSwapChain>        swapChains_;
        HWObjectInstance<NullCommandQueue>      commandQueue_;
        HWObjectContainer<NullCommandBuffer>    commandBuffers_;
        HWObjectContainer<NullBuffer>           buffers_;
        HWObjectContainer<NullBufferArray>      bufferArrays_;
        HWObjectContainer<NullTexture>          textures_;
        HWObjectContainer<NullRenderPass>       renderPasses_;
        HWObjectContainer<NullRenderTarget>     renderTargets_;
        HWObjectContainer<NullShader>           shaders_;
        HWObjectContainer<NullPipelineLayout>   pipelineLayouts_;
        HWObjectContainer<NullPipelineState>    pipelineStates_;
        HWObjectContainer<NullResourceHeap>     resourceHeaps_;
        HWObjectContainer<NullSampler>          samplers_;
        HWObjectContainer<NullQueryHeap>        queryHeaps_;
        HWObjectContainer<NullFence>            fences_;

};


} // /namespace LLGL


#endif



// ================================================================================
