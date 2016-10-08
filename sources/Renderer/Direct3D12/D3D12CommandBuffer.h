/*
 * D3D12CommandBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_D3D12_COMMAND_BUFFER_H__
#define __LLGL_D3D12_COMMAND_BUFFER_H__


#include <LLGL/CommandBuffer.h>
#include <cstddef>
#include "../ComPtr.h"
#include "../DXCommon/DXCore.h"
#include "RenderState/D3D12StateManager.h"

#include <d3d12.h>
#include <dxgi1_4.h>


namespace LLGL
{


class D3D12RenderSystem;

class D3D12CommandBuffer : public CommandBuffer
{

    public:

        /* ----- Common ----- */

        D3D12CommandBuffer(D3D12RenderSystem& renderSystem);

        /* ----- Configuration ----- */

        void SetGraphicsAPIDependentState(const GraphicsAPIDependentStateDescriptor& state) override;

        void SetViewport(const Viewport& viewport) override;
        void SetViewportArray(unsigned int numViewports, const Viewport* viewportArray) override;

        void SetScissor(const Scissor& scissor) override;
        void SetScissorArray(unsigned int numScissors, const Scissor* scissorArray) override;

        void SetClearColor(const ColorRGBAf& color) override;
        void SetClearDepth(float depth) override;
        void SetClearStencil(int stencil) override;

        void ClearBuffers(long flags) override;

        /* ----- Hardware Buffers ------ */

        void SetVertexBuffer(Buffer& buffer) override;
        void SetVertexBufferArray(BufferArray& bufferArray) override;

        void SetIndexBuffer(Buffer& buffer) override;
        
        void SetConstantBuffer(Buffer& buffer, unsigned int slot, long shaderStageFlags = ShaderStageFlags::AllStages) override;
        void SetConstantBufferArray(BufferArray& bufferArray, unsigned int startSlot, long shaderStageFlags = ShaderStageFlags::AllStages) override;
        
        void SetStorageBuffer(Buffer& buffer, unsigned int slot) override;

        /* ----- Textures ----- */

        void SetTexture(Texture& texture, unsigned int slot, long shaderStageFlags = ShaderStageFlags::AllStages) override;
        void SetTextureArray(TextureArray& textureArray, unsigned int startSlot, long shaderStageFlags = ShaderStageFlags::AllStages) override;

        /* ----- Sampler States ----- */

        void SetSampler(Sampler& sampler, unsigned int slot, long shaderStageFlags = ShaderStageFlags::AllStages) override;

        /* ----- Render Targets ----- */

        void SetRenderTarget(RenderTarget& renderTarget) override;
        void SetRenderTarget(RenderContext& renderContext) override;

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

        void DispatchCompute(unsigned int groupSizeX, unsigned int groupSizeY, unsigned int groupSizeZ) override;

        /* ----- Misc ----- */

        void SyncGPU() override;

    private:

        static const UINT maxNumBuffers = 3;

        void InitStateManager();

        //! Sets the current back buffer as render target view.
        void SetBackBufferRTV();

        void ExecuteCommandList();
        void SubmitConsistentStates();

        D3D12RenderSystem&                  renderSystem_;  // reference to its render system

        std::unique_ptr<D3D12StateManager>  stateMngr_;

        ComPtr<ID3D12DescriptorHeap>        rtvDescHeap_;
        UINT                                rtvDescSize_                    = 0;

        UINT64                              fenceValues_[maxNumBuffers]     = { 0 };

        ComPtr<ID3D12CommandAllocator>      commandAlloc_;
        ComPtr<ID3D12GraphicsCommandList>   commandList_;

        UINT                                numFrames_                      = 0;
        UINT                                currentFrame_                   = 0;

        D3DClearState                       clearState_;

};


} // /namespace LLGL


#endif



// ================================================================================
