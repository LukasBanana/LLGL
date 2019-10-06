/*
 * D3D11RenderSystem.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D11_RENDER_SYSTEM_H
#define LLGL_D3D11_RENDER_SYSTEM_H


#include <LLGL/RenderSystem.h>
#include <LLGL/VideoAdapter.h>

#include "D3D11CommandQueue.h"
#include "D3D11CommandBuffer.h"
#include "D3D11RenderContext.h"

#include "Buffer/D3D11Buffer.h"
#include "Buffer/D3D11BufferArray.h"

#include "RenderState/D3D11PipelineState.h"
#include "RenderState/D3D11StateManager.h"
#include "RenderState/D3D11QueryHeap.h"
#include "RenderState/D3D11Fence.h"
#include "RenderState/D3D11ResourceHeap.h"
#include "RenderState/D3D11RenderPass.h"
#include "RenderState/D3D11PipelineLayout.h"

#include "Shader/D3D11Shader.h"
#include "Shader/D3D11ShaderProgram.h"

#include "Texture/D3D11Texture.h"
#include "Texture/D3D11Sampler.h"
#include "Texture/D3D11RenderTarget.h"

#include "../ContainerTypes.h"
#include "../DXCommon/ComPtr.h"

#include <dxgi.h>
#include "Direct3D11.h"


namespace LLGL
{


class D3D11RenderSystem final : public RenderSystem
{

    public:

        /* ----- Common ----- */

        D3D11RenderSystem();
        ~D3D11RenderSystem();

        /* ----- Render Context ------ */

        RenderContext* CreateRenderContext(const RenderContextDescriptor& desc, const std::shared_ptr<Surface>& surface = nullptr) override;

        void Release(RenderContext& renderContext) override;

        /* ----- Command queues ----- */

        CommandQueue* GetCommandQueue() override;

        /* ----- Command buffers ----- */

        CommandBuffer* CreateCommandBuffer(const CommandBufferDescriptor& desc = {}) override;

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
        void ReadTexture(Texture& texture, const TextureRegion& textureRegion, const DstImageDescriptor& imageDesc) override;

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

        PipelineState* CreatePipelineState(const Blob& serializedCache) override;
        PipelineState* CreatePipelineState(const GraphicsPipelineDescriptor& desc, std::unique_ptr<Blob>* serializedCache = nullptr) override;
        PipelineState* CreatePipelineState(const ComputePipelineDescriptor& desc, std::unique_ptr<Blob>* serializedCache = nullptr) override;

        void Release(PipelineState& pipelineState) override;

        /* ----- Queries ----- */

        QueryHeap* CreateQueryHeap(const QueryHeapDescriptor& desc) override;

        void Release(QueryHeap& queryHeap) override;

        /* ----- Fences ----- */

        Fence* CreateFence() override;

        void Release(Fence& fence) override;

    public:

        /* ----- Internal functions ----- */

        // Returns a sample descriptor for the specified format.
        static DXGI_SAMPLE_DESC FindSuitableSampleDesc(ID3D11Device* device, DXGI_FORMAT format, UINT maxSampleCount);

        // Returns the least common denominator of a suitable sample descriptor for all formats.
        static DXGI_SAMPLE_DESC FindSuitableSampleDesc(ID3D11Device* device, std::size_t numFormats, const DXGI_FORMAT* formats, UINT maxSampleCount);

        // Returns the ID3D11Device object.
        inline ID3D11Device* GetDevice() const
        {
            return device_.Get();
        }

        // Returns the selected device feature level.
        inline D3D_FEATURE_LEVEL GetFeatureLevel() const
        {
            return featureLevel_;
        }

    private:

        void CreateFactory();
        void QueryVideoAdapters();
        void CreateDevice(IDXGIAdapter* adapter);
        bool CreateDeviceWithFlags(IDXGIAdapter* adapter, const std::vector<D3D_FEATURE_LEVEL>& featureLevels, UINT flags, HRESULT& hr);
        void CreateStateManagerAndCommandQueue();

        void QueryRendererInfo();
        void QueryRenderingCaps();

        // Returns the minor version of Direct3D 11.X.
        int GetMinorVersion() const;

        void CreateAndInitializeGpuTexture1D(D3D11Texture& textureD3D, const TextureDescriptor& desc, const SrcImageDescriptor* imageDesc);
        void CreateAndInitializeGpuTexture2D(D3D11Texture& textureD3D, const TextureDescriptor& desc, const SrcImageDescriptor* imageDesc);
        void CreateAndInitializeGpuTexture3D(D3D11Texture& textureD3D, const TextureDescriptor& desc, const SrcImageDescriptor* imageDesc);
        void CreateAndInitializeGpuTexture2DMS(D3D11Texture& textureD3D, const TextureDescriptor& desc);

        void UpdateGenericTexture(
            Texture&                    texture,
            std::uint32_t               mipLevel,
            std::uint32_t               arrayLayer,
            const D3D11_BOX&            region,
            const SrcImageDescriptor&   imageDesc
        );

        void InitializeGpuTexture(
            D3D11Texture&               textureD3D,
            const TextureDescriptor&    textureDesc,
            const SrcImageDescriptor*   imageDesc
        );

        void InitializeGpuTextureWithImage(
            D3D11Texture&       textureD3D,
            const Format        format,
            const Extent3D&     extent,
            std::uint32_t       arrayLayers,
            SrcImageDescriptor  imageDesc
        );

        void InitializeGpuTextureWithClearValue(
            D3D11Texture&       textureD3D,
            const Format        format,
            const Extent3D&     extent,
            std::uint32_t       arrayLayers,
            const ClearValue&   clearValue
        );

    private:

        /* ----- Common objects ----- */

        ComPtr<IDXGIFactory>                    factory_;

        ComPtr<ID3D11Device>                    device_;

        #if LLGL_D3D11_ENABLE_FEATURELEVEL >= 1
        ComPtr<ID3D11Device1>                   device1_;
        #endif

        #if LLGL_D3D11_ENABLE_FEATURELEVEL >= 2
        ComPtr<ID3D11Device2>                   device2_;
        #endif

        #if LLGL_D3D11_ENABLE_FEATURELEVEL >= 3
        ComPtr<ID3D11Device3>                   device3_;
        #endif

        ComPtr<ID3D11DeviceContext>             context_;

        D3D_FEATURE_LEVEL                       featureLevel_           = D3D_FEATURE_LEVEL_9_1;

        std::shared_ptr<D3D11StateManager>      stateMngr_;

        /* ----- Hardware object containers ----- */

        HWObjectContainer<D3D11RenderContext>   renderContexts_;
        HWObjectInstance<D3D11CommandQueue>     commandQueue_;
        HWObjectContainer<D3D11CommandBuffer>   commandBuffers_;
        HWObjectContainer<D3D11Buffer>          buffers_;
        HWObjectContainer<D3D11BufferArray>     bufferArrays_;
        HWObjectContainer<D3D11Texture>         textures_;
        HWObjectContainer<D3D11Sampler>         samplers_;
        HWObjectContainer<D3D11RenderPass>      renderPasses_;
        HWObjectContainer<D3D11RenderTarget>    renderTargets_;
        HWObjectContainer<D3D11Shader>          shaders_;
        HWObjectContainer<D3D11ShaderProgram>   shaderPrograms_;
        HWObjectContainer<D3D11PipelineLayout>  pipelineLayouts_;
        HWObjectContainer<D3D11PipelineState>   pipelineStates_;
        HWObjectContainer<D3D11ResourceHeap>    resourceHeaps_;
        HWObjectContainer<D3D11QueryHeap>       queryHeaps_;
        HWObjectContainer<D3D11Fence>           fences_;

        /* ----- Other members ----- */

        std::vector<VideoAdapterDescriptor>     videoAdatperDescs_;

        CPUAccess                               mappedBufferCPUAccess_  = CPUAccess::ReadOnly; // <-- TODO: move this into <D3D11Buffer> class

};


} // /namespace LLGL


#endif



// ================================================================================
