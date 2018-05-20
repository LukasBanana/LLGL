/*
 * D3D12CommandBuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12CommandBuffer.h"
#include "D3D12RenderContext.h"
#include "D3D12RenderSystem.h"
#include "D3D12Types.h"
#include "../CheckedCast.h"
#include "../../Core/Helper.h"
#include <algorithm>
#include "D3DX12/d3dx12.h"

#include "Buffer/D3D12VertexBuffer.h"
#include "Buffer/D3D12VertexBufferArray.h"
#include "Buffer/D3D12IndexBuffer.h"
#include "Buffer/D3D12ConstantBuffer.h"
#include "Buffer/D3D12StorageBuffer.h"

#include "Texture/D3D12Texture.h"


namespace LLGL
{


D3D12CommandBuffer::D3D12CommandBuffer(D3D12RenderSystem& renderSystem)
{
    CreateDevices(renderSystem);
}

/* ----- Configuration ----- */

void D3D12CommandBuffer::SetGraphicsAPIDependentState(const void* stateDesc, std::size_t stateDescSize)
{
    // dummy
}

/* ----- Viewport and Scissor ----- */

void D3D12CommandBuffer::SetViewport(const Viewport& viewport)
{
    D3D12CommandBuffer::SetViewports(1, &viewport);
}

void D3D12CommandBuffer::SetViewports(std::uint32_t numViewports, const Viewport* viewports)
{
    numViewports = std::min(numViewports, std::uint32_t(D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE));

    /* Check if D3D12_VIEWPORT and Viewport structures can be safely reinterpret-casted */
    if ( sizeof(D3D12_VIEWPORT)             == sizeof(Viewport)             &&
         offsetof(D3D12_VIEWPORT, TopLeftX) == offsetof(Viewport, x       ) &&
         offsetof(D3D12_VIEWPORT, TopLeftY) == offsetof(Viewport, y       ) &&
         offsetof(D3D12_VIEWPORT, Width   ) == offsetof(Viewport, width   ) &&
         offsetof(D3D12_VIEWPORT, Height  ) == offsetof(Viewport, height  ) &&
         offsetof(D3D12_VIEWPORT, MinDepth) == offsetof(Viewport, minDepth) &&
         offsetof(D3D12_VIEWPORT, MaxDepth) == offsetof(Viewport, maxDepth) )
    {
        /* Now it's safe to reinterpret cast the viewports into D3D viewports */
        commandList_->RSSetViewports(numViewports, reinterpret_cast<const D3D12_VIEWPORT*>(viewports));
    }
    else
    {
        /* Convert viewport into D3D viewport */
        D3D12_VIEWPORT viewportsD3D[D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];

        for (std::uint32_t i = 0; i < numViewports; ++i)
        {
            const auto& src = viewports[i];
            auto& dest = viewportsD3D[i];

            dest.TopLeftX   = src.x;
            dest.TopLeftY   = src.y;
            dest.Width      = src.width;
            dest.Height     = src.height;
            dest.MinDepth   = src.minDepth;
            dest.MaxDepth   = src.maxDepth;
        }

        commandList_->RSSetViewports(numViewports, viewportsD3D);
    }

    /* If scissor test is disabled, update remaining scissor rectangles to extent of active framebuffer */
    if (!scissorEnabled_)
        SetScissorRectsWithFramebufferExtent(numViewports);
}

void D3D12CommandBuffer::SetScissor(const Scissor& scissor)
{
    D3D12CommandBuffer::SetScissors(1, &scissor);
}

void D3D12CommandBuffer::SetScissors(std::uint32_t numScissors, const Scissor* scissors)
{
    if (scissorEnabled_)
    {
        numScissors = std::min(numScissors, std::uint32_t(D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE));

        D3D12_RECT scissorsD3D[D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];

        for (std::uint32_t i = 0; i < numScissors; ++i)
        {
            const auto& src = scissors[i];
            auto& dest = scissorsD3D[i];

            dest.left   = src.x;
            dest.top    = src.y;
            dest.right  = src.x + src.width;
            dest.bottom = src.y + src.height;
        }

        commandList_->RSSetScissorRects(numScissors, scissorsD3D);
    }
}

/* ----- Clear ----- */

void D3D12CommandBuffer::SetClearColor(const ColorRGBAf& color)
{
    clearValue_.color = color;
}

void D3D12CommandBuffer::SetClearDepth(float depth)
{
    clearValue_.depth = depth;
}

void D3D12CommandBuffer::SetClearStencil(std::uint32_t stencil)
{
    clearValue_.stencil = (stencil & 0xff);
}

void D3D12CommandBuffer::Clear(long flags)
{
    /* Clear color buffer */
    if ((flags & ClearFlags::Color) != 0)
        commandList_->ClearRenderTargetView(rtvDescHandle_, clearValue_.color.Ptr(), 0, nullptr);

    /* Clear depth-stencil buffer */
    INT dsvClearFlags = 0;

    if ((flags & ClearFlags::Depth) != 0)
        dsvClearFlags |= D3D12_CLEAR_FLAG_DEPTH;
    if ((flags & ClearFlags::Stencil) != 0)
        dsvClearFlags |= D3D12_CLEAR_FLAG_STENCIL;

    if (dsvClearFlags)
    {
        commandList_->ClearDepthStencilView(
            rtvDescHandle_,
            static_cast<D3D12_CLEAR_FLAGS>(dsvClearFlags),
            clearValue_.depth,
            static_cast<UINT8>(clearValue_.stencil),
            0,
            nullptr
        );
    }
}

void D3D12CommandBuffer::ClearAttachments(std::uint32_t numAttachments, const AttachmentClear* attachments)
{
    //TODO...
    //CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvDescHandle_, targetIndex, rtvDescHandleSize_);
    //commandList_->ClearRenderTargetView(rtvDescHandle_, clearState_.color.Ptr(), 0, nullptr);
}

/* ----- Buffers ------ */

void D3D12CommandBuffer::SetVertexBuffer(Buffer& buffer)
{
    auto& vertexBufferD3D = LLGL_CAST(D3D12VertexBuffer&, buffer);
    commandList_->IASetVertexBuffers(0, 1, &(vertexBufferD3D.GetView()));
}

void D3D12CommandBuffer::SetVertexBufferArray(BufferArray& bufferArray)
{
    auto& vertexBufferArrayD3D = LLGL_CAST(D3D12VertexBufferArray&, bufferArray);
    commandList_->IASetVertexBuffers(
        0,
        static_cast<UINT>(vertexBufferArrayD3D.GetViews().size()),
        vertexBufferArrayD3D.GetViews().data()
    );
}

void D3D12CommandBuffer::SetIndexBuffer(Buffer& buffer)
{
    auto& indexBufferD3D = LLGL_CAST(D3D12IndexBuffer&, buffer);
    commandList_->IASetIndexBuffer(&(indexBufferD3D.GetView()));
}

#if 0
void D3D12CommandBuffer::SetConstantBuffer(Buffer& buffer, std::uint32_t slot, long shaderStageFlags)
{
    auto& constantBufferD3D = LLGL_CAST(D3D12ConstantBuffer&, buffer);

    /* Set CBV descriptor heap */
    ID3D12DescriptorHeap* descHeaps[1] = { constantBufferD3D.GetDescriptorHeap() };
    commandList_->SetDescriptorHeaps(1, descHeaps);
    commandList_->SetGraphicsRootDescriptorTable(0, descHeaps[0]->GetGPUDescriptorHandleForHeapStart());
}
#endif

/* ----- Stream Output Buffers ------ */

void D3D12CommandBuffer::SetStreamOutputBuffer(Buffer& buffer)
{
    //todo...
}

void D3D12CommandBuffer::SetStreamOutputBufferArray(BufferArray& bufferArray)
{
    //todo...
}

void D3D12CommandBuffer::BeginStreamOutput(const PrimitiveType primitiveType)
{
    // dummy
}

void D3D12CommandBuffer::EndStreamOutput()
{
    // dummy
}

#if 0
/* ----- Textures ----- */

void D3D12CommandBuffer::SetTexture(Texture& texture, std::uint32_t slot, long shaderStageFlags)
{
    auto& textureD3D = LLGL_CAST(D3D12Texture&, texture);

    /* Set SRV descriptor heap */
    ID3D12DescriptorHeap* descHeaps[1] = { textureD3D.GetDescriptorHeap() };
    commandList_->SetDescriptorHeaps(1, descHeaps);
    commandList_->SetGraphicsRootDescriptorTable(0, descHeaps[0]->GetGPUDescriptorHandleForHeapStart());
}
#endif

/* ----- Render Targets ----- */

void D3D12CommandBuffer::SetRenderTarget(RenderTarget& renderTarget)
{
    //todo
}

void D3D12CommandBuffer::SetRenderTarget(RenderContext& renderContext)
{
    auto& renderContextD3D = LLGL_CAST(D3D12RenderContext&, renderContext);

    renderContextD3D.SetCommandBuffer(this);

    /* Set back-buffer RTVs */
    SetBackBufferRTV(renderContextD3D);

    /* Store framebuffer extent */
    const auto& framebufferExtent = renderContextD3D.GetVideoMode().resolution;
    framebufferWidth_   = static_cast<LONG>(framebufferExtent.width);
    framebufferHeight_  = static_cast<LONG>(framebufferExtent.height);

    /* Reset information about default scissor rectangles */
    numBoundScissorRects_ = 0;
}

/* ----- Pipeline States ----- */

void D3D12CommandBuffer::SetGraphicsPipeline(GraphicsPipeline& graphicsPipeline)
{
    /* Set graphics root signature, graphics pipeline state, and primitive topology */
    auto& graphicsPipelineD3D = LLGL_CAST(D3D12GraphicsPipeline&, graphicsPipeline);

    commandList_->SetGraphicsRootSignature(graphicsPipelineD3D.GetRootSignature());
    commandList_->SetPipelineState(graphicsPipelineD3D.GetPipelineState());
    commandList_->IASetPrimitiveTopology(graphicsPipelineD3D.GetPrimitiveTopology());

    /* Scissor rectangle must be updated (if scissor test is disabled) */
    scissorEnabled_ = graphicsPipelineD3D.IsScissorEnabled();
    if (!scissorEnabled_)
        SetScissorRectsWithFramebufferExtent(1);
}

void D3D12CommandBuffer::SetComputePipeline(ComputePipeline& computePipeline)
{
    //todo
}

/* ----- Queries ----- */

void D3D12CommandBuffer::BeginQuery(Query& query)
{
    //todo
}

void D3D12CommandBuffer::EndQuery(Query& query)
{
    //todo
}

bool D3D12CommandBuffer::QueryResult(Query& query, std::uint64_t& result)
{
    return false; //todo
}

bool D3D12CommandBuffer::QueryPipelineStatisticsResult(Query& query, QueryPipelineStatistics& result)
{
    return false; //todo
}

void D3D12CommandBuffer::BeginRenderCondition(Query& query, const RenderConditionMode mode)
{
    //auto predicateOp = (mode >= RenderConditionMode::WaitInverted ? D3D12_PREDICATION_OP_EQUAL_NOT_ZERO : D3D12_PREDICATION_OP_EQUAL_ZERO);
    //commandList_->SetPredication(nullptr, offset, predicateOp);
    //todo...
}

void D3D12CommandBuffer::EndRenderCondition()
{
    //commandList_->SetPredication(nullptr, offset, D3D12_PREDICATION_OP_EQUAL_ZERO);
    //todo...
}

/* ----- Drawing ----- */

void D3D12CommandBuffer::Draw(std::uint32_t numVertices, std::uint32_t firstVertex)
{
    commandList_->DrawInstanced(numVertices, 1, firstVertex, 0);
}

void D3D12CommandBuffer::DrawIndexed(std::uint32_t numVertices, std::uint32_t firstIndex)
{
    commandList_->DrawIndexedInstanced(numVertices, 1, firstIndex, 0, 0);
}

void D3D12CommandBuffer::DrawIndexed(std::uint32_t numVertices, std::uint32_t firstIndex, std::int32_t vertexOffset)
{
    commandList_->DrawIndexedInstanced(numVertices, 1, firstIndex, vertexOffset, 0);
}

void D3D12CommandBuffer::DrawInstanced(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances)
{
    commandList_->DrawInstanced(numVertices, numInstances, firstVertex, 0);
}

void D3D12CommandBuffer::DrawInstanced(std::uint32_t numVertices, std::uint32_t firstVertex, std::uint32_t numInstances, std::uint32_t instanceOffset)
{
    commandList_->DrawInstanced(numVertices, numInstances, firstVertex, instanceOffset);
}

void D3D12CommandBuffer::DrawIndexedInstanced(std::uint32_t numVertices, std::uint32_t numInstances, std::uint32_t firstIndex)
{
    commandList_->DrawIndexedInstanced(numVertices, numInstances, firstIndex, 0, 0);
}

void D3D12CommandBuffer::DrawIndexedInstanced(std::uint32_t numVertices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset)
{
    commandList_->DrawIndexedInstanced(numVertices, numInstances, firstIndex, vertexOffset, 0);
}

void D3D12CommandBuffer::DrawIndexedInstanced(std::uint32_t numVertices, std::uint32_t numInstances, std::uint32_t firstIndex, std::int32_t vertexOffset, std::uint32_t instanceOffset)
{
    commandList_->DrawIndexedInstanced(numVertices, numInstances, firstIndex, vertexOffset, instanceOffset);
}

/* ----- Compute ----- */

void D3D12CommandBuffer::Dispatch(std::uint32_t groupSizeX, std::uint32_t groupSizeY, std::uint32_t groupSizeZ)
{
    commandList_->Dispatch(groupSizeX, groupSizeY, groupSizeZ);
}

/* ----- Extended functions ----- */

void D3D12CommandBuffer::ResetCommandList(ID3D12CommandAllocator* commandAlloc, ID3D12PipelineState* pipelineState)
{
    /* Reset commanb list with command allocator and pipeline state */
    auto hr = commandList_->Reset(commandAlloc, pipelineState);
    DXThrowIfFailed(hr, "failed to reset D3D12 command list");
}


/*
 * ======= Private: =======
 */

void D3D12CommandBuffer::CreateDevices(D3D12RenderSystem& renderSystem)
{
    /* Create command allocator and graphics command list */
    commandAlloc_   = renderSystem.CreateDXCommandAllocator();
    commandList_    = renderSystem.CreateDXCommandList(commandAlloc_.Get());
}

void D3D12CommandBuffer::SetBackBufferRTV(D3D12RenderContext& renderContextD3D)
{
    if (!renderContextD3D.HasMultiSampling())
    {
        /* Indicate that the back buffer will be used as render target */
        renderContextD3D.TransitionRenderTarget(
            D3D12_RESOURCE_STATE_PRESENT,
            D3D12_RESOURCE_STATE_RENDER_TARGET
        );
    }

    /* Set current back buffer as RTV */
    rtvDescHandle_ = renderContextD3D.GetCurrentRTVDescHandle();

    commandList_->OMSetRenderTargets(1, &rtvDescHandle_, FALSE, nullptr);
}

void D3D12CommandBuffer::SetScissorRectsWithFramebufferExtent(UINT numScissorRects)
{
    numScissorRects = std::min(numScissorRects, UINT(D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE));

    if (numScissorRects > numBoundScissorRects_)
    {
        /* Set scissor to render target resolution */
        D3D12_RECT scissorRects[D3D12_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];

        for (UINT i = 0; i < numScissorRects; ++i)
        {
            scissorRects[i].left    = 0;
            scissorRects[i].top     = 0;
            scissorRects[i].right   = framebufferWidth_;
            scissorRects[i].bottom  = framebufferHeight_;
        }

        commandList_->RSSetScissorRects(numScissorRects, scissorRects);

        /* Store new number of bound scissor rectangles */
        numBoundScissorRects_ = numScissorRects;
    }
}


} // /namespace LLGL



// ================================================================================
