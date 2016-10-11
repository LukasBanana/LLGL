/*
 * D3D12CommandBuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12CommandBuffer.h"
//#include "D3D12RenderContext.h"
#include "D3D12RenderSystem.h"
#include "D3D12Types.h"
#include "../CheckedCast.h"
#include "../../Core/Helper.h"
#include <algorithm>
#include "D3DX12/d3dx12.h"

#include "Buffer/D3D12VertexBuffer.h"
#include "Buffer/D3D12IndexBuffer.h"
#include "Buffer/D3D12ConstantBuffer.h"
#include "Buffer/D3D12StorageBuffer.h"


namespace LLGL
{


D3D12CommandBuffer::D3D12CommandBuffer(D3D12RenderSystem& renderSystem) :
    renderSystem_( renderSystem )
{
    InitStateManager();
}

/* ----- Configuration ----- */

void D3D12CommandBuffer::SetGraphicsAPIDependentState(const GraphicsAPIDependentStateDescriptor& state)
{
    // dummy
}

void D3D12CommandBuffer::SetViewport(const Viewport& viewport)
{
    stateMngr_->SetViewports(1, &viewport);
    stateMngr_->SubmitViewports();
}

void D3D12CommandBuffer::SetViewportArray(unsigned int numViewports, const Viewport* viewportArray)
{
    stateMngr_->SetViewports(numViewports, viewportArray);
    stateMngr_->SubmitViewports();
}

void D3D12CommandBuffer::SetScissor(const Scissor& scissor)
{
    stateMngr_->SetScissors(1, &scissor);
    stateMngr_->SubmitScissors();
}

void D3D12CommandBuffer::SetScissorArray(unsigned int numScissors, const Scissor* scissorArray)
{
    stateMngr_->SetScissors(numScissors, scissorArray);
    stateMngr_->SubmitScissors();
}

void D3D12CommandBuffer::SetClearColor(const ColorRGBAf& color)
{
    clearState_.color = color;
}

void D3D12CommandBuffer::SetClearDepth(float depth)
{
    clearState_.depth = depth;
}

void D3D12CommandBuffer::SetClearStencil(int stencil)
{
    clearState_.stencil = stencil;
}

void D3D12CommandBuffer::ClearBuffers(long flags)
{
    /* Get RTV descriptor handle for current frame */
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
        rtvDescHeap_->GetCPUDescriptorHandleForHeapStart(), currentFrame_, rtvDescSize_
    );

    /* Clear color buffer */
    if ((flags & ClearBuffersFlags::Color) != 0)
        commandList_->ClearRenderTargetView(rtvHandle, clearState_.color.Ptr(), 0, nullptr);
    
    /* Clear depth-stencil buffer */
    int dsvClearFlags = 0;

    if ((flags & ClearBuffersFlags::Depth) != 0)
        dsvClearFlags |= D3D12_CLEAR_FLAG_DEPTH;
    if ((flags & ClearBuffersFlags::Stencil) != 0)
        dsvClearFlags |= D3D12_CLEAR_FLAG_STENCIL;
        
    if (dsvClearFlags)
    {
        commandList_->ClearDepthStencilView(
            rtvHandle, static_cast<D3D12_CLEAR_FLAGS>(dsvClearFlags), clearState_.depth, clearState_.stencil, 0, nullptr
        );
    }
}

/* ----- Buffers ------ */

void D3D12CommandBuffer::SetVertexBuffer(Buffer& buffer)
{
    auto& vertexBufferD3D = LLGL_CAST(D3D12VertexBuffer&, buffer);
    commandList_->IASetVertexBuffers(0, 1, &(vertexBufferD3D.GetView()));
}

void D3D12CommandBuffer::SetVertexBufferArray(BufferArray& bufferArray)
{
    //todo...
}

void D3D12CommandBuffer::SetIndexBuffer(Buffer& buffer)
{
    auto& indexBufferD3D = LLGL_CAST(D3D12IndexBuffer&, buffer);
    commandList_->IASetIndexBuffer(&(indexBufferD3D.GetView()));
}

void D3D12CommandBuffer::SetConstantBuffer(Buffer& buffer, unsigned int slot, long shaderStageFlags)
{
    auto& constantBufferD3D = LLGL_CAST(D3D12ConstantBuffer&, buffer);

    /* Set CBV descriptor heap */
    ID3D12DescriptorHeap* descHeaps[] = { constantBufferD3D.GetDescriptorHeap() };
    commandList_->SetDescriptorHeaps(1, descHeaps);
    commandList_->SetGraphicsRootDescriptorTable(0, descHeaps[0]->GetGPUDescriptorHandleForHeapStart());
}

void D3D12CommandBuffer::SetConstantBufferArray(BufferArray& bufferArray, unsigned int startSlot, long shaderStageFlags)
{
    //todo...
}

void D3D12CommandBuffer::SetStorageBuffer(Buffer& buffer, unsigned int slot, long shaderStageFlags)
{
    //todo...
}

void D3D12CommandBuffer::SetStorageBufferArray(BufferArray& bufferArray, unsigned int startSlot, long shaderStageFlags)
{
    //todo...
}

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

/* ----- Textures ----- */

void D3D12CommandBuffer::SetTexture(Texture& texture, unsigned int slot, long shaderStageFlags)
{
    //todo
}

void D3D12CommandBuffer::SetTextureArray(TextureArray& textureArray, unsigned int startSlot, long shaderStageFlags)
{
    //todo
}

/* ----- Sampler States ----- */

void D3D12CommandBuffer::SetSampler(Sampler& sampler, unsigned int slot, long shaderStageFlags)
{
    //todo
}

/* ----- Render Targets ----- */

void D3D12CommandBuffer::SetRenderTarget(RenderTarget& renderTarget)
{
    //todo
}

void D3D12CommandBuffer::SetRenderTarget(RenderContext& renderContext)
{
    //todo
}

/* ----- Pipeline States ----- */

void D3D12CommandBuffer::SetGraphicsPipeline(GraphicsPipeline& graphicsPipeline)
{
    /* Set graphics root signature, graphics pipeline state, and primitive topology */
    auto& graphicsPipelineD3D = LLGL_CAST(D3D12GraphicsPipeline&, graphicsPipeline);
    commandList_->SetGraphicsRootSignature(graphicsPipelineD3D.GetRootSignature());
    commandList_->SetPipelineState(graphicsPipelineD3D.GetPipelineState());
    commandList_->IASetPrimitiveTopology(graphicsPipelineD3D.GetPrimitiveTopology());
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

void D3D12CommandBuffer::Draw(unsigned int numVertices, unsigned int firstVertex)
{
    commandList_->DrawInstanced(numVertices, 1, firstVertex, 0);
}

void D3D12CommandBuffer::DrawIndexed(unsigned int numVertices, unsigned int firstIndex)
{
    commandList_->DrawIndexedInstanced(numVertices, 1, firstIndex, 0, 0);
}

void D3D12CommandBuffer::DrawIndexed(unsigned int numVertices, unsigned int firstIndex, int vertexOffset)
{
    commandList_->DrawIndexedInstanced(numVertices, 1, firstIndex, vertexOffset, 0);
}

void D3D12CommandBuffer::DrawInstanced(unsigned int numVertices, unsigned int firstVertex, unsigned int numInstances)
{
    commandList_->DrawInstanced(numVertices, numInstances, firstVertex, 0);
}

void D3D12CommandBuffer::DrawInstanced(unsigned int numVertices, unsigned int firstVertex, unsigned int numInstances, unsigned int instanceOffset)
{
    commandList_->DrawInstanced(numVertices, numInstances, firstVertex, instanceOffset);
}

void D3D12CommandBuffer::DrawIndexedInstanced(unsigned int numVertices, unsigned int numInstances, unsigned int firstIndex)
{
    commandList_->DrawIndexedInstanced(numVertices, numInstances, firstIndex, 0, 0);
}

void D3D12CommandBuffer::DrawIndexedInstanced(unsigned int numVertices, unsigned int numInstances, unsigned int firstIndex, int vertexOffset)
{
    commandList_->DrawIndexedInstanced(numVertices, numInstances, firstIndex, vertexOffset, 0);
}

void D3D12CommandBuffer::DrawIndexedInstanced(unsigned int numVertices, unsigned int numInstances, unsigned int firstIndex, int vertexOffset, unsigned int instanceOffset)
{
    commandList_->DrawIndexedInstanced(numVertices, numInstances, firstIndex, vertexOffset, instanceOffset);
}

/* ----- Compute ----- */

void D3D12CommandBuffer::DispatchCompute(unsigned int groupSizeX, unsigned int groupSizeY, unsigned int groupSizeZ)
{
    commandList_->Dispatch(groupSizeX, groupSizeY, groupSizeZ);
}

/* ----- Misc ----- */

void D3D12CommandBuffer::SyncGPU()
{
    renderSystem_.SyncGPU(fenceValues_[currentFrame_]);
}


/*
 * ======= Private: =======
 */

void D3D12CommandBuffer::InitStateManager()
{
    /* Create state manager */
    stateMngr_ = MakeUnique<D3D12StateManager>(commandList_);

    /* Initialize states */
    /*auto resolution = desc_.videoMode.resolution;

    Viewport viewport(0.0f, 0.0f, static_cast<float>(resolution.x), static_cast<float>(resolution.y));
    stateMngr_->SetViewports(1, &viewport);

    Scissor scissor(0, 0, resolution.x, resolution.y);
    stateMngr_->SetScissors(1, &scissor);*/
}

void D3D12CommandBuffer::SetBackBufferRTV()
{
    /* Set current back buffer as RTV */
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvDesc(rtvDescHeap_->GetCPUDescriptorHandleForHeapStart(), currentFrame_, rtvDescSize_);
    commandList_->OMSetRenderTargets(1, &rtvDesc, FALSE, nullptr);
}

void D3D12CommandBuffer::ExecuteCommandList()
{
    renderSystem_.CloseAndExecuteCommandList(commandList_.Get());
}

void D3D12CommandBuffer::SubmitConsistentStates()
{
    /* Submit all constistent states: viewports, scissors */
    stateMngr_->SubmitViewports();
    stateMngr_->SubmitScissors();
}


} // /namespace LLGL



// ================================================================================
