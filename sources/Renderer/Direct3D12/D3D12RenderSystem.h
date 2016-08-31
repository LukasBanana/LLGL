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
#include "../ContainerTypes.h"
#include <d3d12.h>
#include <dxgi1_4.h>


namespace LLGL
{


class D3D12RenderSystem : public RenderSystem
{

    public:

        /* ----- Render System ----- */

        D3D12RenderSystem();
        ~D3D12RenderSystem();

        RenderContext* CreateRenderContext(const RenderContextDescriptor& desc, const std::shared_ptr<Window>& window = nullptr) override;

        void Release(RenderContext& renderContext) override;

        /* ----- Hardware Buffers ------ */

        VertexBuffer* CreateVertexBuffer() override;
        IndexBuffer* CreateIndexBuffer() override;
        ConstantBuffer* CreateConstantBuffer() override;

        void Release(VertexBuffer& vertexBuffer) override;
        void Release(IndexBuffer& indexBuffer) override;
        void Release(ConstantBuffer& constantBuffer) override;

        void WriteVertexBuffer(VertexBuffer& vertexBuffer, const void* data, std::size_t dataSize, const BufferUsage usage, const VertexFormat& vertexFormat) override;
        void WriteIndexBuffer(IndexBuffer& indexBuffer, const void* data, std::size_t dataSize, const BufferUsage usage, const IndexFormat& indexFormat) override;
        void WriteConstantBuffer(ConstantBuffer& constantBuffer, const void* data, std::size_t dataSize, const BufferUsage usage) override;

        void WriteVertexBufferSub(VertexBuffer& vertexBuffer, const void* data, std::size_t dataSize, std::size_t offset) override;
        void WriteIndexBufferSub(IndexBuffer& indexBuffer, const void* data, std::size_t dataSize, std::size_t offset) override;
        void WriteConstantBufferSub(ConstantBuffer& constantBuffer, const void* data, std::size_t dataSize, std::size_t offset) override;

        /* ----- Textures ----- */

        Texture* CreateTexture() override;

        void Release(Texture& texture) override;

        TextureDescriptor QueryTextureDescriptor(const Texture& texture) override;

        void WriteTexture1D(Texture& texture, const TextureFormat format, int size, const ImageDataDescriptor* imageDesc = nullptr) override;
        void WriteTexture2D(Texture& texture, const TextureFormat format, const Gs::Vector2i& size, const ImageDataDescriptor* imageDesc = nullptr) override;
        void WriteTexture3D(Texture& texture, const TextureFormat format, const Gs::Vector3i& size, const ImageDataDescriptor* imageDesc = nullptr) override;
        void WriteTextureCube(Texture& texture, const TextureFormat format, const Gs::Vector2i& size, const ImageDataDescriptor* imageDesc = nullptr) override;
        void WriteTexture1DArray(Texture& texture, const TextureFormat format, int size, unsigned int layers, const ImageDataDescriptor* imageDesc = nullptr) override;
        void WriteTexture2DArray(Texture& texture, const TextureFormat format, const Gs::Vector2i& size, unsigned int layers, const ImageDataDescriptor* imageDesc = nullptr) override;
        void WriteTextureCubeArray(Texture& texture, const TextureFormat format, const Gs::Vector2i& size, unsigned int layers, const ImageDataDescriptor* imageDesc = nullptr) override;
        
        void WriteTexture1DSub(Texture& texture, int mipLevel, int position, int size, const ImageDataDescriptor& imageDesc) override;
        void WriteTexture2DSub(Texture& texture, int mipLevel, const Gs::Vector2i& position, const Gs::Vector2i& size, const ImageDataDescriptor& imageDesc) override;
        void WriteTexture3DSub(Texture& texture, int mipLevel, const Gs::Vector3i& position, const Gs::Vector3i& size, const ImageDataDescriptor& imageDesc) override;
        void WriteTextureCubeSub(Texture& texture, int mipLevel, const Gs::Vector2i& position, const AxisDirection cubeFace, const Gs::Vector2i& size, const ImageDataDescriptor& imageDesc) override;
        void WriteTexture1DArraySub(Texture& texture, int mipLevel, int position, unsigned int layers, int size, const ImageDataDescriptor& imageDesc) override;
        void WriteTexture2DArraySub(Texture& texture, int mipLevel, const Gs::Vector2i& position, unsigned int layers, const Gs::Vector2i& size, const ImageDataDescriptor& imageDesc) override;
        void WriteTextureCubeArraySub(Texture& texture, int mipLevel, const Gs::Vector2i& position, unsigned int layers, const AxisDirection cubeFace, const Gs::Vector2i& size, const ImageDataDescriptor& imageDesc) override;

        void ReadTexture(const Texture& texture, int mipLevel, ColorFormat dataFormat, DataType dataType, void* data) override;

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
        //ComputePipeline* CreateComputePipeline(const ComputePipelineDescriptor& desc) override;
        
        void Release(GraphicsPipeline& graphicsPipeline) override;

        /* ----- Extended internal functions ----- */

        ID3D12CommandQueue* CreateCommandQueue();
        ID3D12CommandAllocator* CreateCommandAllocator();
        ID3D12Fence* CreateFence(UINT64 initialValue);

    private:

        void CreateFactory();
        void QueryVideoAdapters();
        void CreateDevice();
        bool CreateDevice(HRESULT& hr, IDXGIAdapter* adapter, const std::vector<D3D_FEATURE_LEVEL>& featureLevels);

        /* ----- Common D3D objects ----- */

        IDXGIFactory4*                              factory_        = nullptr;
        ID3D12Device*                               device_         = nullptr;
        ID3D12CommandQueue*                         cmdQueue_       = nullptr;
        ID3D12DescriptorHeap*                       descHeap_       = nullptr;

        /* ----- Hardware object containers ----- */

        HWObjectContainer<D3D12RenderContext>       renderContexts_;
        
        /*HWObjectContainer<D3D12VertexBuffer>        vertexBuffers_;
        HWObjectContainer<D3D12IndexBuffer>         indexBuffers_;
        HWObjectContainer<D3D12ConstantBuffer>      constantBuffers_;

        HWObjectContainer<D3D12Texture>             textures_;
        HWObjectContainer<D3D12RenderTarget>        renderTargets_;

        HWObjectContainer<D3D12Shader>              shaders_;
        HWObjectContainer<D3D12ShaderProgram>       shaderPrograms_;

        HWObjectContainer<D3D12GraphicsPipeline>    graphicsPipelines_;
        HWObjectContainer<D3D12Sampler>             samplers_;*/

        /* ----- Other members ----- */

        std::vector<VideoAdapterDescriptor>         videoAdatperDescs_;

};


} // /namespace LLGL


#endif



// ================================================================================
