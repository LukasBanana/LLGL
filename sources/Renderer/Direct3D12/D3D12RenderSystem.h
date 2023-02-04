/*
 * D3D12RenderSystem.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D12_RENDER_SYSTEM_H
#define LLGL_D3D12_RENDER_SYSTEM_H


#include <LLGL/RenderSystem.h>
#include <LLGL/VideoAdapter.h>

#include "D3D12Device.h"
#include "D3D12SwapChain.h"
#include "Command/D3D12CommandQueue.h"
#include "Command/D3D12CommandBuffer.h"
#include "Command/D3D12CommandContext.h"
#include "Command/D3D12SignatureFactory.h"

#include "Buffer/D3D12Buffer.h"
#include "Buffer/D3D12StagingBufferPool.h"

#include "Texture/D3D12Texture.h"
#include "Texture/D3D12Sampler.h"
#include "Texture/D3D12RenderTarget.h"

#include "RenderState/D3D12Fence.h"
#include "RenderState/D3D12PipelineState.h"
#include "RenderState/D3D12PipelineLayout.h"
#include "RenderState/D3D12ResourceHeap.h"
#include "RenderState/D3D12RenderPass.h"
#include "RenderState/D3D12QueryHeap.h"

#include "Shader/D3D12Shader.h"

#include "../ContainerTypes.h"
#include "../DXCommon/ComPtr.h"
#include <d3d12.h>
#include <dxgi1_4.h>


namespace LLGL
{


class D3D12RenderSystem final : public RenderSystem
{

    public:

        /* ----- Common ----- */

        D3D12RenderSystem();
        ~D3D12RenderSystem();

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
        PipelineState* CreatePipelineState(const GraphicsPipelineDescriptor& pipelineStateDesc, std::unique_ptr<Blob>* serializedCache = nullptr) override;
        PipelineState* CreatePipelineState(const ComputePipelineDescriptor& pipelineStateDesc, std::unique_ptr<Blob>* serializedCache = nullptr) override;

        void Release(PipelineState& pipelineState) override;

        /* ----- Queries ----- */

        QueryHeap* CreateQueryHeap(const QueryHeapDescriptor& queryHeapDesc) override;

        void Release(QueryHeap& queryHeap) override;

        /* ----- Fences ----- */

        Fence* CreateFence() override;

        void Release(Fence& fence) override;

    public:

        /* ----- Extended internal functions ----- */

        ComPtr<IDXGISwapChain1> CreateDXSwapChain(const DXGI_SWAP_CHAIN_DESC1& swapChainDescDXGI, HWND wnd);

        // Internal fence
        void SignalFenceValue(UINT64& fenceValue);
        void WaitForFenceValue(UINT64 fenceValue);

        // Waits until the GPU has done all previous work.
        void SyncGPU(UINT64& fenceValue);
        void SyncGPU();

        // Updates the image data of the specified texture region.
        void UpdateGpuTexture(
            D3D12Texture&               textureD3D,
            const TextureRegion&        region,
            const SrcImageDescriptor&   imageDesc,
            ComPtr<ID3D12Resource>&     uploadBuffer
        );

        // Maps the range of the specified D3D buffer between GPU and CPU memory space.
        void* MapBufferRange(D3D12Buffer& bufferD3D, const CPUAccess access, std::uint64_t offset, std::uint64_t size);

        // Returns the feature level of the D3D device.
        inline D3D_FEATURE_LEVEL GetFeatureLevel() const
        {
            return device_.GetFeatureLevel();
        }

        // Returns the native ID3D12Device object.
        inline ID3D12Device* GetDXDevice() const
        {
            return device_.GetNative();
        }

        // Returns the device object.
        inline D3D12Device& GetDevice()
        {
            return device_;
        }

        // Returns the device object.
        inline const D3D12Device& GetDevice() const
        {
            return device_;
        }

        // Returns the command signmature factory.
        inline const D3D12SignatureFactory& GetSignatureFactory() const
        {
            return cmdSignatureFactory_;
        }

    private:

        #ifdef LLGL_DEBUG
        void EnableDebugLayer();
        #endif

        void CreateFactory();
        void QueryVideoAdapters();
        void CreateDevice();

        void QueryRendererInfo();
        void QueryRenderingCaps();

        // Close, execute, and reset command list.
        void ExecuteCommandList();
        void ExecuteCommandListAndSync();

        std::unique_ptr<D3D12Buffer> CreateGpuBuffer(const BufferDescriptor& bufferDesc, const void* initialData);

        const D3D12RenderPass* GetDefaultRenderPass() const;

    private:

        /* ----- Common objects ----- */

        ComPtr<IDXGIFactory4>                   factory_;
        D3D12Device                             device_;
        D3D12CommandContext*                    commandContext_         = nullptr;
        D3D12PipelineLayout                     defaultPipelineLayout_;
        D3D12SignatureFactory                   cmdSignatureFactory_;
        D3D12StagingBufferPool                  stagingBufferPool_;

        /* ----- Hardware object containers ----- */

        HWObjectContainer<D3D12SwapChain>       swapChains_;
        HWObjectInstance<D3D12CommandQueue>     commandQueue_;
        HWObjectContainer<D3D12CommandBuffer>   commandBuffers_;
        HWObjectContainer<D3D12Buffer>          buffers_;
        HWObjectContainer<BufferArray>          bufferArrays_;
        HWObjectContainer<D3D12Texture>         textures_;
        HWObjectContainer<D3D12Sampler>         samplers_;
        HWObjectContainer<D3D12RenderPass>      renderPasses_;
        HWObjectContainer<D3D12RenderTarget>    renderTargets_;
        HWObjectContainer<D3D12Shader>          shaders_;
        HWObjectContainer<D3D12PipelineLayout>  pipelineLayouts_;
        HWObjectContainer<D3D12PipelineState>   pipelineStates_;
        HWObjectContainer<D3D12ResourceHeap>    resourceHeaps_;
        HWObjectContainer<D3D12QueryHeap>       queryHeaps_;
        HWObjectContainer<D3D12Fence>           fences_;

        /* ----- Other members ----- */

        std::vector<VideoAdapterDescriptor>     videoAdatperDescs_;

};


} // /namespace LLGL


#endif



// ================================================================================
