/*
 * D3D11RenderContext.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_D3D11_RENDER_CONTEXT_H__
#define __LLGL_D3D11_RENDER_CONTEXT_H__


#include <LLGL/Window.h>
#include <LLGL/RenderContext.h>
#include <cstddef>
#include "../ComPtr.h"
#include "../DXCommon/DXCore.h"
#include <vector>
#include <d3d11.h>
#include <dxgi.h>


namespace LLGL
{


class D3D11RenderSystem;
class D3D11StateManager;

class D3D11RenderContext : public RenderContext
{

    public:

        /* ----- Common ----- */

        D3D11RenderContext(
            D3D11RenderSystem& renderSystem,
            D3D11StateManager& stateMngr,
            const ComPtr<ID3D11DeviceContext>& context,
            RenderContextDescriptor desc,
            const std::shared_ptr<Window>& window
        );

        void Present() override;

        /* ----- Configuration ----- */

        void SetGraphicsAPIDependentState(const GraphicsAPIDependentStateDescriptor& state) override;

        void SetVideoMode(const VideoModeDescriptor& videoModeDesc) override;
        void SetVsync(const VsyncDescriptor& vsyncDesc) override;

        void SetViewports(const std::vector<Viewport>& viewports) override;
        void SetScissors(const std::vector<Scissor>& scissors) override;

        void SetClearColor(const ColorRGBAf& color) override;
        void SetClearDepth(float depth) override;
        void SetClearStencil(int stencil) override;

        void ClearBuffers(long flags) override;

        /* ----- Hardware Buffers ------ */

        void SetVertexBuffer(VertexBuffer& vertexBuffer) override;
        void SetIndexBuffer(IndexBuffer& indexBuffer) override;
        void SetConstantBuffer(ConstantBuffer& constantBuffer, unsigned int slot, long shaderStageFlags = ShaderStageFlags::AllStages) override;
        void SetStorageBuffer(StorageBuffer& storageBuffer, unsigned int slot) override;

        void* MapStorageBuffer(StorageBuffer& storageBuffer, const BufferCPUAccess access) override;
        void UnmapStorageBuffer() override;

        /* ----- Textures ----- */

        void SetTexture(Texture& texture, unsigned int slot) override;

        void GenerateMips(Texture& texture) override;

        /* ----- Sampler States ----- */

        void SetSampler(Sampler& sampler, unsigned int slot) override;

        /* ----- Render Targets ----- */

        void SetRenderTarget(RenderTarget& renderTarget) override;
        void UnsetRenderTarget() override;

        /* ----- Pipeline States ----- */

        void SetGraphicsPipeline(GraphicsPipeline& graphicsPipeline) override;
        void SetComputePipeline(ComputePipeline& computePipeline) override;

        /* ----- Queries ----- */

        void BeginQuery(Query& query) override;
        void EndQuery(Query& query) override;

        bool QueryResult(Query& query, std::uint64_t& result) override;

        void BeginRenderCondition(Query& query, const RenderConditionMode mode) override;
        void EndRenderCondition() override;

        /* ----- Drawing ----- */

        void Draw(unsigned int numVertices, unsigned int firstVertex) override;

        void DrawIndexed(unsigned int numVertices, unsigned int firstIndex) override;
        void DrawIndexed(unsigned int numVertices, unsigned int firstIndex, int vertexOffset) override;

        void DrawInstanced(unsigned int numVertices, unsigned int firstVertex, unsigned int numInstances) override;
        void DrawInstanced(unsigned int numVertices, unsigned int firstVertex, unsigned int numInstances, unsigned int instanceOffset) override;

        void DrawIndexedInstanced(unsigned int numVertices, unsigned int numInstances, unsigned int firstIndex) override;
        void DrawIndexedInstanced(unsigned int numVertices, unsigned int numInstances, unsigned int firstIndex, int vertexOffset) override;
        void DrawIndexedInstanced(unsigned int numVertices, unsigned int numInstances, unsigned int firstIndex, int vertexOffset, unsigned int instanceOffset) override;

        /* ----- Compute ----- */

        void DispatchCompute(const Gs::Vector3ui& threadGroupSize) override;

        /* ----- Misc ----- */

        void SyncGPU() override;

    private:

        struct D3D11BackBuffer
        {
            ComPtr<ID3D11Texture2D>         colorBuffer;
            ComPtr<ID3D11RenderTargetView>  rtv;
            ComPtr<ID3D11Texture2D>         depthStencil;
            ComPtr<ID3D11DepthStencilView>  dsv;
        };

        struct D3D11FramebufferView
        {
            std::vector<ID3D11RenderTargetView*>    rtvList;
            ID3D11DepthStencilView*                 dsv = nullptr;
        };

        void CreateSwapChain();
        void CreateBackBuffer(UINT width, UINT height);
        void ResizeBackBuffer(UINT width, UINT height);

        void SetDefaultRenderTargets();
        void SubmitFramebufferView();

        D3D11RenderSystem&          renderSystem_;  // reference to its render system
        D3D11StateManager&          stateMngr_;
        RenderContextDescriptor     desc_;
        
        ComPtr<ID3D11DeviceContext> context_;

        ComPtr<IDXGISwapChain>      swapChain_;
        UINT                        swapChainInterval_  = 0;

        D3D11BackBuffer             backBuffer_;
        D3D11FramebufferView        framebufferView_;

        D3DClearState               clearState_;

};


} // /namespace LLGL


#endif



// ================================================================================
