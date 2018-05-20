/*
 * D3D12CommandBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D12_COMMAND_BUFFER_H
#define LLGL_D3D12_COMMAND_BUFFER_H


#include <LLGL/CommandBuffer.h>
#include <cstddef>
#include "../DXCommon/ComPtr.h"
#include "../DXCommon/DXCore.h"

#include <d3d12.h>
#include <dxgi1_4.h>


namespace LLGL
{


class D3D12RenderSystem;
class D3D12RenderContext;

class D3D12CommandBuffer : public CommandBuffer
{

    public:

        /* ----- Common ----- */

        D3D12CommandBuffer(D3D12RenderSystem& renderSystem);

        /* ----- Configuration ----- */

        void SetGraphicsAPIDependentState(const void* stateDesc, std::size_t stateDescSize) override;

        /* ----- Viewport and Scissor ----- */

        void SetViewport(const Viewport& viewport) override;
        void SetViewports(std::uint32_t numViewports, const Viewport* viewports) override;

        void SetScissor(const Scissor& scissor) override;
        void SetScissors(std::uint32_t numScissors, const Scissor* scissors) override;

        /* ----- Clear ----- */

        void SetClearColor(const ColorRGBAf& color) override;
        void SetClearDepth(float depth) override;
        void SetClearStencil(std::uint32_t stencil) override;

        void Clear(long flags) override;
        void ClearAttachments(std::uint32_t numAttachments, const AttachmentClear* attachments) override;

        /* ----- Input Assembly ------ */

        void SetVertexBuffer(Buffer& buffer) override;
        void SetVertexBufferArray(BufferArray& bufferArray) override;

        void SetIndexBuffer(Buffer& buffer) override;

        /* ----- Stream Output Buffers ------ */

        void SetStreamOutputBuffer(Buffer& buffer) override;
        void SetStreamOutputBufferArray(BufferArray& bufferArray) override;

        void BeginStreamOutput(const PrimitiveType primitiveType) override;
        void EndStreamOutput() override;

        /* ----- Resource Views ----- */

        //void SetResourceViewHeaps(std::uint32_t numHeaps, ResourceViewHeap* const * heapArray);

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
        bool QueryPipelineStatisticsResult(Query& query, QueryPipelineStatistics& result) override;

        void BeginRenderCondition(Query& query, const RenderConditionMode mode) override;
        void EndRenderCondition() override;

        /* ----- Drawing ----- */

        void Draw(std::uint32_t numVertices, std::uint32_t firstVertex) override;

        void DrawIndexed(std::uint32_t numVertices, std::uint32_t firstIndex) override;
        void DrawIndexed(std::uint32_t numVertices, std::uint32_t firstIndex, std::int32_t vertexOffset) override;

        void DrawInstanced(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances) override;
        void DrawInstanced(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances, std::uint32_t instanceOffset) override;

        void DrawIndexedInstanced(std::uint32_t numVertices, std::uint32_t numInstances, std::uint32_t firstIndex) override;
        void DrawIndexedInstanced(std::uint32_t numVertices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset) override;
        void DrawIndexedInstanced(std::uint32_t numVertices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset, std::uint32_t instanceOffset) override;

        /* ----- Compute ----- */

        void Dispatch(std::uint32_t groupSizeX, std::uint32_t groupSizeY, std::uint32_t groupSizeZ) override;

        /* ----- Extended functions ----- */

        inline ID3D12GraphicsCommandList* GetCommandList() const
        {
            return commandList_.Get();
        }

        void ResetCommandList(ID3D12CommandAllocator* commandAlloc, ID3D12PipelineState* pipelineState);

    private:

        static const UINT maxNumBuffers = 3;

        void CreateDevices(D3D12RenderSystem& renderSystem);

        // Sets the current back buffer as render target view.
        void SetBackBufferRTV(D3D12RenderContext& renderContextD3D);

        ComPtr<ID3D12CommandAllocator>      commandAlloc_;
        ComPtr<ID3D12GraphicsCommandList>   commandList_;

        D3D12_CPU_DESCRIPTOR_HANDLE         rtvDescHandle_;

        //UINT64                              fenceValues_[maxNumBuffers] = { 0 };

        ClearValue                          clearValue_;

};


} // /namespace LLGL


#endif



// ================================================================================
