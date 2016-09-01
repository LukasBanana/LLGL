/*
 * D3D12RenderContext.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_D3D12_RENDER_CONTEXT_H__
#define __LLGL_D3D12_RENDER_CONTEXT_H__


#include <LLGL/Window.h>
#include <LLGL/RenderContext.h>
#include <cstddef>

//#include "RenderState/D3D12StateManager.h"

#include <d3d12.h>
#include <dxgi1_4.h>


namespace LLGL
{


class D3D12RenderSystem;

class D3D12RenderContext : public RenderContext
{

    public:

        /* ----- Common ----- */

        D3D12RenderContext(
            D3D12RenderSystem& renderSystem,
            RenderContextDescriptor desc,
            const std::shared_ptr<Window>& window
        );
        ~D3D12RenderContext();

        std::map<RendererInfo, std::string> QueryRendererInfo() const override;

        RenderingCaps QueryRenderingCaps() const override;

        ShadingLanguage QueryShadingLanguage() const override;

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

        void SetDrawMode(const DrawMode drawMode) override;

        /* ----- Hardware Buffers ------ */

        void BindVertexBuffer(VertexBuffer& vertexBuffer) override;
        void UnbindVertexBuffer() override;

        void BindIndexBuffer(IndexBuffer& indexBuffer) override;
        void UnbindIndexBuffer() override;

        void BindConstantBuffer(ConstantBuffer& constantBuffer, unsigned int index) override;
        void UnbindConstantBuffer(unsigned int index) override;

        /* ----- Textures ----- */

        void BindTexture(unsigned int layer, Texture& texture) override;
        void UnbindTexture(unsigned int layer) override;

        void GenerateMips(Texture& texture) override;

        /* ----- Sampler States ----- */

        void BindSampler(unsigned int layer, Sampler& sampler) override;
        void UnbindSampler(unsigned int layer) override;

        /* ----- Render Targets ----- */

        void BindRenderTarget(RenderTarget& renderTarget) override;
        void UnbindRenderTarget() override;

        /* ----- Pipeline States ----- */

        void BindGraphicsPipeline(GraphicsPipeline& graphicsPipeline) override;
        //void BindComputePipeline(ComputePipeline& computePipeline) override;

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

    private:

        static const UINT maxNumBuffers = 3;

        void SyncGPU();
        void CreateWindowSizeDependentResources();

        D3D12RenderSystem&                  renderSystem_;  // reference to its render system
        RenderContextDescriptor             desc_;

        //std::shared_ptr<D3D12StateManager>  stateMngr_;

        IDXGISwapChain1*                    swapChain_                      = nullptr;
        ID3D12DescriptorHeap*               descHeap_                       = nullptr;

        ID3D12CommandAllocator*             cmdAllocs_[maxNumBuffers]       = { nullptr };
        ID3D12Resource*                     renderTargets_[maxNumBuffers]   = { nullptr };
        UINT64                              fenceValues_[maxNumBuffers]     = { 0 };

        UINT                                numFrames_                      = 0;
        UINT                                currentFrame_                   = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
