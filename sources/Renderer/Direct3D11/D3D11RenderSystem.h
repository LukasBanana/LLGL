/*
 * D3D11RenderSystem.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_D3D11_RENDER_SYSTEM_H__
#define __LLGL_D3D11_RENDER_SYSTEM_H__


#include <LLGL/RenderSystem.h>
#include <LLGL/VideoAdapter.h>
#include "D3D11RenderContext.h"

#include "Buffer/D3D11VertexBuffer.h"
#include "Buffer/D3D11IndexBuffer.h"
#include "Buffer/D3D11ConstantBuffer.h"
//#include "Buffer/D3D11StorageBuffer.h"

#include "RenderState/D3D11GraphicsPipeline.h"
#include "RenderState/D3D11StateManager.h"

#include "Shader/D3D11Shader.h"
#include "Shader/D3D11ShaderProgram.h"

#include "../ContainerTypes.h"
#include "../ComPtr.h"
#include <d3d11.h>
#include <dxgi.h>


namespace LLGL
{


class D3D11RenderSystem : public RenderSystem
{

    public:

        /* ----- Common ----- */

        D3D11RenderSystem();
        ~D3D11RenderSystem();

        std::map<RendererInfo, std::string> QueryRendererInfo() const override;

        RenderingCaps QueryRenderingCaps() const override;

        ShadingLanguage QueryShadingLanguage() const override;

        /* ----- Render Context ------ */

        RenderContext* CreateRenderContext(const RenderContextDescriptor& desc, const std::shared_ptr<Window>& window = nullptr) override;

        void Release(RenderContext& renderContext) override;

        /* ----- Hardware Buffers ------ */

        VertexBuffer* CreateVertexBuffer() override;
        IndexBuffer* CreateIndexBuffer() override;
        ConstantBuffer* CreateConstantBuffer() override;
        StorageBuffer* CreateStorageBuffer() override;

        void Release(VertexBuffer& vertexBuffer) override;
        void Release(IndexBuffer& indexBuffer) override;
        void Release(ConstantBuffer& constantBuffer) override;
        void Release(StorageBuffer& storageBuffer) override;

        void SetupVertexBuffer(VertexBuffer& vertexBuffer, const void* data, std::size_t dataSize, const BufferUsage usage, const VertexFormat& vertexFormat) override;
        void SetupIndexBuffer(IndexBuffer& indexBuffer, const void* data, std::size_t dataSize, const BufferUsage usage, const IndexFormat& indexFormat) override;
        void SetupConstantBuffer(ConstantBuffer& constantBuffer, const void* data, std::size_t dataSize, const BufferUsage usage) override;
        void SetupStorageBuffer(StorageBuffer& storageBuffer, const void* data, std::size_t dataSize, const BufferUsage usage) override;

        void WriteVertexBuffer(VertexBuffer& vertexBuffer, const void* data, std::size_t dataSize, std::size_t offset) override;
        void WriteIndexBuffer(IndexBuffer& indexBuffer, const void* data, std::size_t dataSize, std::size_t offset) override;
        void WriteConstantBuffer(ConstantBuffer& constantBuffer, const void* data, std::size_t dataSize, std::size_t offset) override;
        void WriteStorageBuffer(StorageBuffer& storageBuffer, const void* data, std::size_t dataSize, std::size_t offset) override;

        /* ----- Textures ----- */

        Texture* CreateTexture() override;

        void Release(Texture& texture) override;

        TextureDescriptor QueryTextureDescriptor(const Texture& texture) override;

        void SetupTexture1D(Texture& texture, const TextureFormat format, int size, const ImageDataDescriptor* imageDesc = nullptr) override;
        void SetupTexture2D(Texture& texture, const TextureFormat format, const Gs::Vector2i& size, const ImageDataDescriptor* imageDesc = nullptr) override;
        void SetupTexture3D(Texture& texture, const TextureFormat format, const Gs::Vector3i& size, const ImageDataDescriptor* imageDesc = nullptr) override;
        void SetupTextureCube(Texture& texture, const TextureFormat format, const Gs::Vector2i& size, const ImageDataDescriptor* imageDesc = nullptr) override;
        void SetupTexture1DArray(Texture& texture, const TextureFormat format, int size, unsigned int layers, const ImageDataDescriptor* imageDesc = nullptr) override;
        void SetupTexture2DArray(Texture& texture, const TextureFormat format, const Gs::Vector2i& size, unsigned int layers, const ImageDataDescriptor* imageDesc = nullptr) override;
        void SetupTextureCubeArray(Texture& texture, const TextureFormat format, const Gs::Vector2i& size, unsigned int layers, const ImageDataDescriptor* imageDesc = nullptr) override;
        
        void WriteTexture1D(Texture& texture, int mipLevel, int position, int size, const ImageDataDescriptor& imageDesc) override;
        void WriteTexture2D(Texture& texture, int mipLevel, const Gs::Vector2i& position, const Gs::Vector2i& size, const ImageDataDescriptor& imageDesc) override;
        void WriteTexture3D(Texture& texture, int mipLevel, const Gs::Vector3i& position, const Gs::Vector3i& size, const ImageDataDescriptor& imageDesc) override;

        void WriteTextureCube(
            Texture& texture, int mipLevel, const Gs::Vector2i& position, const AxisDirection cubeFace,
            const Gs::Vector2i& size, const ImageDataDescriptor& imageDesc
        ) override;
        
        void WriteTexture1DArray(
            Texture& texture, int mipLevel, int position, unsigned int layerOffset,
            int size, unsigned int layers, const ImageDataDescriptor& imageDesc
        ) override;
        
        void WriteTexture2DArray(
            Texture& texture, int mipLevel, const Gs::Vector2i& position, unsigned int layerOffset,
            const Gs::Vector2i& size, unsigned int layers, const ImageDataDescriptor& imageDesc
        ) override;

        void WriteTextureCubeArray(
            Texture& texture, int mipLevel, const Gs::Vector2i& position, unsigned int layerOffset, const AxisDirection cubeFaceOffset,
            const Gs::Vector2i& size, unsigned int cubeFaces, const ImageDataDescriptor& imageDesc
        ) override;

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
        ComputePipeline* CreateComputePipeline(const ComputePipelineDescriptor& desc) override;
        
        void Release(GraphicsPipeline& graphicsPipeline) override;
        void Release(ComputePipeline& computePipeline) override;

        /* ----- Queries ----- */

        Query* CreateQuery(const QueryType type) override;

        void Release(Query& query) override;

        /* ----- Extended internal functions ----- */

        ComPtr<IDXGISwapChain> CreateDXSwapChain(DXGI_SWAP_CHAIN_DESC& desc);

        inline D3D_FEATURE_LEVEL GetFeatureLevel() const
        {
            return featureLevel_;
        }

        inline ID3D11Device* GetDevice() const
        {
            return device_.Get();
        }

    private:
        
        void CreateFactory();
        //void QueryVideoAdapters();
        void CreateDevice(IDXGIAdapter* adapter);
        void InitStateManager();

        /* ----- Common objects ----- */

        ComPtr<IDXGIFactory>                        factory_;
        ComPtr<ID3D11Device>                        device_;
        ComPtr<ID3D11DeviceContext>                 context_;
        D3D_FEATURE_LEVEL                           featureLevel_           = D3D_FEATURE_LEVEL_9_1;

        std::unique_ptr<D3D11StateManager>          stateMngr_;

        /* ----- Hardware object containers ----- */

        HWObjectContainer<D3D11RenderContext>       renderContexts_;
        
        HWObjectContainer<D3D11VertexBuffer>        vertexBuffers_;
        HWObjectContainer<D3D11IndexBuffer>         indexBuffers_;
        HWObjectContainer<D3D11ConstantBuffer>      constantBuffers_;
        //HWObjectContainer<D3D11StorageBuffer>       storageBuffers_;

        //HWObjectContainer<D3D11Texture>             textures_;
        //HWObjectContainer<D3D11RenderTarget>        renderTargets_;

        HWObjectContainer<D3D11Shader>              shaders_;
        HWObjectContainer<D3D11ShaderProgram>       shaderPrograms_;

        HWObjectContainer<D3D11GraphicsPipeline>    graphicsPipelines_;
        //HWObjectContainer<D3D11Sampler>             samplers_;

        /* ----- Other members ----- */

        std::vector<VideoAdapterDescriptor>         videoAdatperDescs_;

};


} // /namespace LLGL


#endif



// ================================================================================
