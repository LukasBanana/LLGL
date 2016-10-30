/*
 * D3D11RenderSystem.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D11_RENDER_SYSTEM_H
#define LLGL_D3D11_RENDER_SYSTEM_H


#include <LLGL/RenderSystem.h>
#include <LLGL/VideoAdapter.h>

#include "D3D11CommandBuffer.h"
#include "D3D11RenderContext.h"

#include "Buffer/D3D11Buffer.h"
#include "Buffer/D3D11BufferArray.h"

#include "RenderState/D3D11GraphicsPipeline.h"
#include "RenderState/D3D11ComputePipeline.h"
#include "RenderState/D3D11StateManager.h"
#include "RenderState/D3D11Query.h"

#include "Shader/D3D11Shader.h"
#include "Shader/D3D11ShaderProgram.h"

#include "Texture/D3D11Texture.h"
#include "Texture/D3D11TextureArray.h"
#include "Texture/D3D11Sampler.h"
#include "Texture/D3D11SamplerArray.h"
#include "Texture/D3D11RenderTarget.h"

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

        /* ----- Render Context ------ */

        RenderContext* CreateRenderContext(const RenderContextDescriptor& desc, const std::shared_ptr<Surface>& surface = nullptr) override;

        void Release(RenderContext& renderContext) override;

        /* ----- Command buffers ----- */

        CommandBuffer* CreateCommandBuffer() override;

        void Release(CommandBuffer& commandBuffer) override;

        /* ----- Buffers ------ */

        Buffer* CreateBuffer(const BufferDescriptor& desc, const void* initialData = nullptr) override;
        BufferArray* CreateBufferArray(unsigned int numBuffers, Buffer* const * bufferArray) override;

        void Release(Buffer& buffer) override;
        void Release(BufferArray& bufferArray) override;
        
        void WriteBuffer(Buffer& buffer, const void* data, std::size_t dataSize, std::size_t offset) override;

        void* MapBuffer(Buffer& buffer, const BufferCPUAccess access) override;
        void UnmapBuffer(Buffer& buffer) override;

        /* ----- Textures ----- */

        Texture* CreateTexture(const TextureDescriptor& textureDesc, const ImageDescriptor* imageDesc = nullptr) override;
        TextureArray* CreateTextureArray(unsigned int numTextures, Texture* const * textureArray) override;

        void Release(Texture& texture) override;
        void Release(TextureArray& textureArray) override;

        TextureDescriptor QueryTextureDescriptor(const Texture& texture) override;
        
        void WriteTexture(Texture& texture, const SubTextureDescriptor& subTextureDesc, const ImageDescriptor& imageDesc) override;

        void ReadTexture(const Texture& texture, int mipLevel, ImageFormat imageFormat, DataType dataType, void* buffer) override;

        void GenerateMips(Texture& texture) override;

        /* ----- Sampler States ---- */

        Sampler* CreateSampler(const SamplerDescriptor& desc) override;
        SamplerArray* CreateSamplerArray(unsigned int numSamplers, Sampler* const * samplerArray) override;

        void Release(Sampler& sampler) override;
        void Release(SamplerArray& samplerArray) override;

        /* ----- Render Targets ----- */

        RenderTarget* CreateRenderTarget(const RenderTargetDescriptor& desc) override;

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
        void QueryVideoAdapters();
        void CreateDevice(IDXGIAdapter* adapter);
        void InitStateManager();

        void QueryRendererInfo();
        void QueryRenderingCaps();

        void BuildGenericTexture1D(D3D11Texture& textureD3D, const TextureDescriptor& descD3D, const ImageDescriptor* imageDesc, UINT miscFlags);
        void BuildGenericTexture2D(D3D11Texture& textureD3D, const TextureDescriptor& descD3D, const ImageDescriptor* imageDesc, UINT miscFlags);
        void BuildGenericTexture3D(D3D11Texture& textureD3D, const TextureDescriptor& descD3D, const ImageDescriptor* imageDesc, UINT miscFlags);
        void BuildGenericTexture2DMS(D3D11Texture& textureD3D, const TextureDescriptor& descD3D);
        
        void UpdateGenericTexture(
            Texture& texture, unsigned int mipLevel, unsigned int layer,
            const Gs::Vector3ui& position, const Gs::Vector3ui& size,
            const ImageDescriptor& imageDesc
        );

        /* ----- Common objects ----- */

        ComPtr<IDXGIFactory>                        factory_;
        ComPtr<ID3D11Device>                        device_;
        ComPtr<ID3D11DeviceContext>                 context_;
        D3D_FEATURE_LEVEL                           featureLevel_ = D3D_FEATURE_LEVEL_9_1;

        std::unique_ptr<D3D11StateManager>          stateMngr_;

        /* ----- Hardware object containers ----- */

        HWObjectContainer<D3D11RenderContext>       renderContexts_;
        HWObjectContainer<D3D11CommandBuffer>       commandBuffers_;
        HWObjectContainer<D3D11Buffer>              buffers_;
        HWObjectContainer<D3D11BufferArray>         bufferArrays_;
        HWObjectContainer<D3D11Texture>             textures_;
        HWObjectContainer<D3D11TextureArray>        textureArrays_;
        HWObjectContainer<D3D11Sampler>             samplers_;
        HWObjectContainer<D3D11SamplerArray>        samplerArrays_;
        HWObjectContainer<D3D11RenderTarget>        renderTargets_;
        HWObjectContainer<D3D11Shader>              shaders_;
        HWObjectContainer<D3D11ShaderProgram>       shaderPrograms_;
        HWObjectContainer<D3D11GraphicsPipeline>    graphicsPipelines_;
        HWObjectContainer<D3D11ComputePipeline>     computePipelines_;
        HWObjectContainer<D3D11Query>               queries_;

        /* ----- Other members ----- */

        std::vector<VideoAdapterDescriptor>         videoAdatperDescs_;

        BufferCPUAccess                             mappedBufferCPUAccess_  = BufferCPUAccess::ReadOnly;

};


} // /namespace LLGL


#endif



// ================================================================================
