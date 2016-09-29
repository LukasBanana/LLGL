/*
 * D3D12RenderSystem.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_D3D12_RENDER_SYSTEM_H__
#define __LLGL_D3D12_RENDER_SYSTEM_H__


#include <LLGL/RenderSystem.h>
#include <LLGL/VideoAdapter.h>
#include "D3D12RenderContext.h"

#include "Buffer/D3D12VertexBuffer.h"
#include "Buffer/D3D12IndexBuffer.h"
#include "Buffer/D3D12ConstantBuffer.h"
#include "Buffer/D3D12StorageBuffer.h"

#include "RenderState/D3D12GraphicsPipeline.h"

#include "Shader/D3D12Shader.h"
#include "Shader/D3D12ShaderProgram.h"

#include "../ContainerTypes.h"
#include "../ComPtr.h"
#include <d3d12.h>
#include <dxgi1_4.h>


namespace LLGL
{


class D3D12RenderSystem : public RenderSystem
{

    public:

        /* ----- Common ----- */

        D3D12RenderSystem();
        ~D3D12RenderSystem();

        std::map<RendererInfo, std::string> QueryRendererInfo() const override;

        RenderingCaps QueryRenderingCaps() const override;

        ShadingLanguage QueryShadingLanguage() const override;

        /* ----- Render Context ------ */

        RenderContext* CreateRenderContext(const RenderContextDescriptor& desc, const std::shared_ptr<Window>& window = nullptr) override;

        void Release(RenderContext& renderContext) override;

        /* ----- Hardware Buffers ------ */

        VertexBuffer* CreateVertexBuffer(const VertexBufferDescriptor& desc, const void* initialData = nullptr) override;
        IndexBuffer* CreateIndexBuffer(const IndexBufferDescriptor& desc, const void* initialData = nullptr) override;
        ConstantBuffer* CreateConstantBuffer(const ConstantBufferDescriptor& desc, const void* initialData = nullptr) override;
        StorageBuffer* CreateStorageBuffer(std::size_t size, const BufferUsage usage, const void* initialData = nullptr) override;

        void Release(VertexBuffer& vertexBuffer) override;
        void Release(IndexBuffer& indexBuffer) override;
        void Release(ConstantBuffer& constantBuffer) override;
        void Release(StorageBuffer& storageBuffer) override;

        void WriteVertexBuffer(VertexBuffer& vertexBuffer, const void* data, std::size_t dataSize, std::size_t offset) override;
        void WriteIndexBuffer(IndexBuffer& indexBuffer, const void* data, std::size_t dataSize, std::size_t offset) override;
        void WriteConstantBuffer(ConstantBuffer& constantBuffer, const void* data, std::size_t dataSize, std::size_t offset) override;
        void WriteStorageBuffer(StorageBuffer& storageBuffer, const void* data, std::size_t dataSize, std::size_t offset) override;

        /* ----- Textures ----- */

        Texture* CreateTexture(const TextureDescriptor& textureDesc, const ImageDescriptor* imageDesc = nullptr) override;

        void Release(Texture& texture) override;

        TextureDescriptor QueryTextureDescriptor(const Texture& texture) override;
        
        void WriteTexture(Texture& texture, const SubTextureDescriptor& subTextureDesc, const ImageDescriptor& imageDesc) override;

        void ReadTexture(const Texture& texture, int mipLevel, ImageFormat imageFormat, DataType dataType, void* buffer) override;

        void GenerateMips(Texture& texture) override;

        /* ----- Sampler States ---- */

        Sampler* CreateSampler(const SamplerDescriptor& desc) override;

        void Release(Sampler& sampler) override;

        /* ----- Render Targets ----- */

        RenderTarget* CreateRenderTarget(unsigned int multiSamples = 0) override;

        void Release(RenderTarget& renderTarget) override;

        /* ----- Shader ----- */

        Shader* CreateShader(const ShaderType type) override;
        ShaderProgram* CreateShaderProgram() override;

        void Release(Shader& shader) override;
        void Release(ShaderProgram& shaderProgram) override;

        /* ----- Pipeline States ----- */

        GraphicsPipeline* CreateGraphicsPipeline(const GraphicsPipelineDescriptor& desc) override;
        ComputePipeline* CreateComputePipeline(const ComputePipelineDescriptor& desc) override;
        
        void Release(GraphicsPipeline& graphicsPipeline) override;
        void Release(ComputePipeline& computePipeline) override;

        /* ----- Queries ----- */

        Query* CreateQuery(const QueryDescriptor& desc) override;

        void Release(Query& query) override;

        /* ----- Extended internal functions ----- */

        ComPtr<IDXGISwapChain1> CreateDXSwapChain(const DXGI_SWAP_CHAIN_DESC1& desc, HWND wnd);
        ComPtr<ID3D12CommandQueue> CreateDXCommandQueue();
        ComPtr<ID3D12CommandAllocator> CreateDXCommandAllocator();
        ComPtr<ID3D12GraphicsCommandList> CreateDXCommandList(ID3D12CommandAllocator* commandAlloc);
        ComPtr<ID3D12PipelineState> CreateDXGfxPipelineState(const D3D12_GRAPHICS_PIPELINE_STATE_DESC& desc);
        ComPtr<ID3D12DescriptorHeap> CreateDXDescriptorHeap(const D3D12_DESCRIPTOR_HEAP_DESC& desc);

        void CloseAndExecuteCommandList(ID3D12GraphicsCommandList* commandList);

        //! Waits until the GPU has done all previous work.
        void SyncGPU(UINT64& fenceValue);
        void SyncGPU();

        void SignalFenceValue(UINT64 fenceValue);
        void WaitForFenceValue(UINT64 fenceValue);

        inline D3D_FEATURE_LEVEL GetFeatureLevel() const
        {
            return featureLevel_;
        }

        inline ID3D12Device* GetDevice() const
        {
            return device_.Get();
        }

        inline ID3D12CommandQueue* GetCommandQueue() const
        {
            return commandQueue_.Get();
        }

    private:
        
        bool OnMakeCurrent(RenderContext* renderContext) override;

        #ifdef LLGL_DEBUG
        void EnableDebugLayer();
        #endif

        void CreateFactory();
        void QueryVideoAdapters();
        void CreateDevice();
        bool CreateDevice(HRESULT& hr, IDXGIAdapter* adapter, const std::vector<D3D_FEATURE_LEVEL>& featureLevels);
        void CreateGPUSynchObjects();

        /* ----- Common objects ----- */

        ComPtr<IDXGIFactory4>                       factory_;
        ComPtr<ID3D12Device>                        device_;
        D3D_FEATURE_LEVEL                           featureLevel_           = D3D_FEATURE_LEVEL_9_1;

        ComPtr<ID3D12CommandQueue>                  commandQueue_;
        ComPtr<ID3D12CommandAllocator>              commandAlloc_;
        ComPtr<ID3D12GraphicsCommandList>           commandList_; // graphics command list to upload data to the GPU

        ComPtr<ID3D12Fence>                         fence_;
        HANDLE                                      fenceEvent_             = 0;
        UINT64                                      fenceValue_             = 0;

        #ifdef LLGL_DEBUG
        //ComPtr<ID3D12Debug>                         debugDevice_;
        //ComPtr<ID3D12InfoQueue>                     debugInfoQueue_;
        #endif

        D3D12RenderContext*                         activeRenderContext_    = nullptr;

        /* ----- Hardware object containers ----- */

        HWObjectContainer<D3D12RenderContext>       renderContexts_;
        
        HWObjectContainer<D3D12VertexBuffer>        vertexBuffers_;
        HWObjectContainer<D3D12IndexBuffer>         indexBuffers_;
        HWObjectContainer<D3D12ConstantBuffer>      constantBuffers_;
        HWObjectContainer<D3D12StorageBuffer>       storageBuffers_;

        //HWObjectContainer<D3D12Texture>             textures_;
        //HWObjectContainer<D3D12RenderTarget>        renderTargets_;

        HWObjectContainer<D3D12Shader>              shaders_;
        HWObjectContainer<D3D12ShaderProgram>       shaderPrograms_;

        HWObjectContainer<D3D12GraphicsPipeline>    graphicsPipelines_;
        //HWObjectContainer<D3D12Sampler>             samplers_;

        /* ----- Other members ----- */

        std::vector<VideoAdapterDescriptor>         videoAdatperDescs_;

};


} // /namespace LLGL


#endif



// ================================================================================
