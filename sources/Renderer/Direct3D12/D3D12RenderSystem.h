/*
 * D3D12RenderSystem.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D12_RENDER_SYSTEM_H
#define LLGL_D3D12_RENDER_SYSTEM_H


#include <LLGL/RenderSystem.h>
#include <LLGL/VideoAdapter.h>

#include "D3D12Device.h"
#include "D3D12CommandQueue.h"
#include "D3D12CommandBuffer.h"
#include "D3D12RenderContext.h"

#include "Buffer/D3D12Buffer.h"
#include "Texture/D3D12Texture.h"
#include "Texture/D3D12Sampler.h"

#include "RenderState/D3D12Fence.h"
#include "RenderState/D3D12GraphicsPipeline.h"
#include "RenderState/D3D12PipelineLayout.h"
#include "RenderState/D3D12ResourceHeap.h"
#include "RenderState/D3D12RenderPass.h"
#include "RenderState/D3D12QueryHeap.h"

#include "Shader/D3D12Shader.h"
#include "Shader/D3D12ShaderProgram.h"

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

        /* ----- Render Context ------ */

        RenderContext* CreateRenderContext(const RenderContextDescriptor& desc, const std::shared_ptr<Surface>& surface = nullptr) override;

        void Release(RenderContext& renderContext) override;

        /* ----- Command queues ----- */

        CommandQueue* GetCommandQueue() override;

        /* ----- Command buffers ----- */

        CommandBuffer* CreateCommandBuffer(const CommandBufferDescriptor& desc = {}) override;
        CommandBufferExt* CreateCommandBufferExt(const CommandBufferDescriptor& desc = {}) override;

        void Release(CommandBuffer& commandBuffer) override;

        /* ----- Buffers ------ */

        Buffer* CreateBuffer(const BufferDescriptor& desc, const void* initialData = nullptr) override;
        BufferArray* CreateBufferArray(std::uint32_t numBuffers, Buffer* const * bufferArray) override;

        void Release(Buffer& buffer) override;
        void Release(BufferArray& bufferArray) override;

        void WriteBuffer(Buffer& dstBuffer, std::uint64_t dstOffset, const void* data, std::uint64_t dataSize) override;

        void* MapBuffer(Buffer& buffer, const CPUAccess access) override;
        void UnmapBuffer(Buffer& buffer) override;

        /* ----- Textures ----- */

        Texture* CreateTexture(const TextureDescriptor& textureDesc, const SrcImageDescriptor* imageDesc = nullptr) override;

        void Release(Texture& texture) override;

        void WriteTexture(Texture& texture, const TextureRegion& textureRegion, const SrcImageDescriptor& imageDesc) override;

        void ReadTexture(const Texture& texture, std::uint32_t mipLevel, const DstImageDescriptor& imageDesc) override;

        void GenerateMips(Texture& texture) override;
        void GenerateMips(Texture& texture, std::uint32_t baseMipLevel, std::uint32_t numMipLevels, std::uint32_t baseArrayLayer = 0, std::uint32_t numArrayLayers = 1) override;

        /* ----- Sampler States ---- */

        Sampler* CreateSampler(const SamplerDescriptor& desc) override;

        void Release(Sampler& sampler) override;

        /* ----- Resource Heaps ----- */

        ResourceHeap* CreateResourceHeap(const ResourceHeapDescriptor& desc) override;

        void Release(ResourceHeap& resourceHeap) override;

        /* ----- Render Passes ----- */

        RenderPass* CreateRenderPass(const RenderPassDescriptor& desc) override;

        void Release(RenderPass& renderPass) override;

        /* ----- Render Targets ----- */

        RenderTarget* CreateRenderTarget(const RenderTargetDescriptor& desc) override;

        void Release(RenderTarget& renderTarget) override;

        /* ----- Shader ----- */

        Shader* CreateShader(const ShaderDescriptor& desc) override;
        ShaderProgram* CreateShaderProgram(const ShaderProgramDescriptor& desc) override;

        void Release(Shader& shader) override;
        void Release(ShaderProgram& shaderProgram) override;

        /* ----- Pipeline Layouts ----- */

        PipelineLayout* CreatePipelineLayout(const PipelineLayoutDescriptor& desc) override;

        void Release(PipelineLayout& pipelineLayout) override;

        /* ----- Pipeline States ----- */

        GraphicsPipeline* CreateGraphicsPipeline(const GraphicsPipelineDescriptor& desc) override;
        ComputePipeline* CreateComputePipeline(const ComputePipelineDescriptor& desc) override;

        void Release(GraphicsPipeline& graphicsPipeline) override;
        void Release(ComputePipeline& computePipeline) override;

        /* ----- Queries ----- */

        QueryHeap* CreateQueryHeap(const QueryHeapDescriptor& desc) override;

        void Release(QueryHeap& queryHeap) override;

        /* ----- Fences ----- */

        Fence* CreateFence() override;

        void Release(Fence& fence) override;

        /* ----- Extended internal functions ----- */

        ComPtr<IDXGISwapChain1> CreateDXSwapChain(const DXGI_SWAP_CHAIN_DESC1& desc, HWND wnd);

        // Internal fence
        void SignalFenceValue(UINT64 fenceValue);
        void WaitForFenceValue(UINT64 fenceValue);

        // Waits until the GPU has done all previous work.
        void SyncGPU(UINT64& fenceValue);
        void SyncGPU();

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

        inline const D3D12Device& GetDevice() const
        {
            return device_;
        }

    private:

        #ifdef LLGL_DEBUG
        void EnableDebugLayer();
        #endif

        void CreateFactory();
        void QueryVideoAdapters();
        void CreateDevice();
        void CreateGPUSynchObjects();

        void QueryRendererInfo();
        void QueryRenderingCaps();

        // Close, execute, and reset command list.
        void ExecuteCommandList();

        std::unique_ptr<D3D12Buffer> MakeBufferAndInitialize(const BufferDescriptor& desc, const void* initialData);

        /* ----- Common objects ----- */

        ComPtr<IDXGIFactory4>                       factory_;
        D3D12Device                                 device_;

        ComPtr<ID3D12CommandAllocator>              graphicsCmdAlloc_;
        ComPtr<ID3D12GraphicsCommandList>           graphicsCmdList_;   // graphics command list to upload data to the GPU
        ComPtr<ID3D12CommandAllocator>              computeCmdAlloc_;
        ComPtr<ID3D12GraphicsCommandList>           computeCmdList_;    // compute command list to generate MIP-maps

        ComPtr<ID3D12Fence>                         fence_;
        HANDLE                                      fenceEvent_             = 0;
        UINT64                                      fenceValue_             = 0;

        D3D12PipelineLayout                         defaultPipelineLayout_;

        #ifdef LLGL_DEBUG
        //ComPtr<ID3D12Debug>                         debugDevice_;
        //ComPtr<ID3D12InfoQueue>                     debugInfoQueue_;
        #endif

        /* ----- Hardware object containers ----- */

        HWObjectContainer<D3D12RenderContext>       renderContexts_;
        HWObjectInstance<D3D12CommandQueue>         commandQueue_;
        HWObjectContainer<D3D12CommandBuffer>       commandBuffers_;
        HWObjectContainer<D3D12Buffer>              buffers_;
        HWObjectContainer<BufferArray>              bufferArrays_;
        HWObjectContainer<D3D12Texture>             textures_;
        HWObjectContainer<D3D12Sampler>             samplers_;
        HWObjectContainer<D3D12RenderPass>          renderPasses_;
        //HWObjectContainer<D3D12RenderTarget>        renderTargets_;
        HWObjectContainer<D3D12Shader>              shaders_;
        HWObjectContainer<D3D12ShaderProgram>       shaderPrograms_;
        HWObjectContainer<D3D12PipelineLayout>      pipelineLayouts_;
        HWObjectContainer<D3D12GraphicsPipeline>    graphicsPipelines_;
        HWObjectContainer<D3D12ResourceHeap>        resourceHeaps_;
        HWObjectContainer<D3D12QueryHeap>           queryHeaps_;
        HWObjectContainer<D3D12Fence>               fences_;

        /* ----- Other members ----- */

        std::vector<VideoAdapterDescriptor>         videoAdatperDescs_;

};


} // /namespace LLGL


#endif



// ================================================================================
