/*
 * D3D12RenderContext.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12RenderContext.h"
#include "D3D12RenderSystem.h"
#include "D3D12Types.h"
#include "../CheckedCast.h"
#include <LLGL/Platform/NativeHandle.h>
#include "../../Core/Helper.h"
#include <algorithm>
#include "D3DX12/d3dx12.h"


namespace LLGL
{


D3D12RenderContext::D3D12RenderContext(
    D3D12RenderSystem& renderSystem,
    RenderContextDescriptor desc,
    const std::shared_ptr<Window>& window) :
        renderSystem_   ( renderSystem ),
        desc_           ( desc         )
{
    /* Setup window for the render context */
    SetWindow(window, desc_.videoMode, nullptr);
    CreateWindowSizeDependentResources();
    InitStateManager();

    /* Initialize v-sync */
    SetVsync(desc_.vsync);
}

void D3D12RenderContext::Present()
{
    /* Execute pending command list */
    ExecuteCommandList();

    /* Indicate that the render target will now be used to present when the command list is done executing */
    auto resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
        renderTargets_[currentFrame_].Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PRESENT
    );

    commandList_->ResourceBarrier(1, &resourceBarrier);

    /* Present swap-chain with vsync interval */
    auto hr = swapChain_->Present(swapChainInterval_, 0);
    DXThrowIfFailed(hr, "failed to present D3D12 swap-chain");

    /* Advance frame counter */
    MoveToNextFrame();

    /* Reset command allocator */
    hr = commandAlloc_->Reset();
    DXThrowIfFailed(hr, "failed to reset D3D12 command allocator");

    hr = commandList_->Reset(commandAlloc_.Get(), nullptr);
    DXThrowIfFailed(hr, "failed to reset D3D12 graphics command list");

    /* Set current back buffer as new render target view */
    SetBackBufferRTV();

    /* Re-submit consistent states */
    SubmitConsistentStates();
}

/* ----- Configuration ----- */

void D3D12RenderContext::SetGraphicsAPIDependentState(const GraphicsAPIDependentStateDescriptor& state)
{
    // dummy
}

void D3D12RenderContext::SetVideoMode(const VideoModeDescriptor& videoModeDesc)
{
    if (GetVideoMode() != videoModeDesc)
    {
        //todo

        /* Update window appearance and store new video mode in base function */
        RenderContext::SetVideoMode(videoModeDesc);
    }
}

void D3D12RenderContext::SetVsync(const VsyncDescriptor& vsyncDesc)
{
    desc_.vsync = vsyncDesc;
    swapChainInterval_ = (vsyncDesc.enabled ? std::max(1u, std::min(vsyncDesc.interval, 4u)) : 0u);
}

void D3D12RenderContext::SetViewports(const std::vector<Viewport>& viewports)
{
    stateMngr_->SetViewports(viewports);
    stateMngr_->SubmitViewports();
}

void D3D12RenderContext::SetScissors(const std::vector<Scissor>& scissors)
{
    stateMngr_->SetScissors(scissors);
    stateMngr_->SubmitScissors();
}

void D3D12RenderContext::SetClearColor(const ColorRGBAf& color)
{
    clearState_.color = color;
}

void D3D12RenderContext::SetClearDepth(float depth)
{
    clearState_.depth = depth;
}

void D3D12RenderContext::SetClearStencil(int stencil)
{
    clearState_.stencil = stencil;
}

void D3D12RenderContext::ClearBuffers(long flags)
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

/* ----- Hardware Buffers ------ */

void D3D12RenderContext::SetVertexBuffer(VertexBuffer& vertexBuffer)
{
    auto& vertexBufferD3D = LLGL_CAST(D3D12VertexBuffer&, vertexBuffer);
    commandList_->IASetVertexBuffers(0, 1, &(vertexBufferD3D.GetView()));
}

void D3D12RenderContext::SetIndexBuffer(IndexBuffer& indexBuffer)
{
    auto& indexBufferD3D = LLGL_CAST(D3D12IndexBuffer&, indexBuffer);
    commandList_->IASetIndexBuffer(&(indexBufferD3D.GetView()));
}

//INCOMPLETE
void D3D12RenderContext::SetConstantBuffer(ConstantBuffer& constantBuffer, unsigned int slot, long shaderStageFlags)
{
    auto& constantBufferD3D = LLGL_CAST(D3D12ConstantBuffer&, constantBuffer);

    /* Set CBV descriptor heap */
    ID3D12DescriptorHeap* descHeaps[] = { constantBufferD3D.GetDescriptorHeap() };
    commandList_->SetDescriptorHeaps(1, descHeaps);
    commandList_->SetGraphicsRootDescriptorTable(0, descHeaps[0]->GetGPUDescriptorHandleForHeapStart());
}

void D3D12RenderContext::SetStorageBuffer(StorageBuffer& storageBuffer, unsigned int slot)
{
    //todo
}

void* D3D12RenderContext::MapStorageBuffer(StorageBuffer& storageBuffer, const BufferCPUAccess access)
{
    return nullptr;//todo
}

void D3D12RenderContext::UnmapStorageBuffer()
{
    //todo
}

/* ----- Textures ----- */

void D3D12RenderContext::SetTexture(Texture& texture, unsigned int slot, long shaderStageFlags)
{
    //todo
}

void D3D12RenderContext::GenerateMips(Texture& texture)
{
    //todo
}

/* ----- Sampler States ----- */

void D3D12RenderContext::SetSampler(Sampler& sampler, unsigned int slot, long shaderStageFlags)
{
    //todo
}

/* ----- Render Targets ----- */

void D3D12RenderContext::SetRenderTarget(RenderTarget& renderTarget)
{
    //todo
}

void D3D12RenderContext::UnsetRenderTarget()
{
    //todo
}

/* ----- Pipeline States ----- */

void D3D12RenderContext::SetGraphicsPipeline(GraphicsPipeline& graphicsPipeline)
{
    /* Set graphics root signature, graphics pipeline state, and primitive topology */
    auto& graphicsPipelineD3D = LLGL_CAST(D3D12GraphicsPipeline&, graphicsPipeline);
    commandList_->SetGraphicsRootSignature(graphicsPipelineD3D.GetRootSignature());
    commandList_->SetPipelineState(graphicsPipelineD3D.GetPipelineState());
    commandList_->IASetPrimitiveTopology(graphicsPipelineD3D.GetPrimitiveTopology());
}

void D3D12RenderContext::SetComputePipeline(ComputePipeline& computePipeline)
{
    //todo
}

/* ----- Queries ----- */

void D3D12RenderContext::BeginQuery(Query& query)
{
    //todo
}

void D3D12RenderContext::EndQuery(Query& query)
{
    //todo
}

bool D3D12RenderContext::QueryResult(Query& query, std::uint64_t& result)
{
    return false; //todo
}

void D3D12RenderContext::BeginRenderCondition(Query& query, const RenderConditionMode mode)
{
    //auto predicateOp = (mode >= RenderConditionMode::WaitInverted ? D3D12_PREDICATION_OP_EQUAL_NOT_ZERO : D3D12_PREDICATION_OP_EQUAL_ZERO);
    //commandList_->SetPredication(nullptr, offset, predicateOp);
    //todo...
}

void D3D12RenderContext::EndRenderCondition()
{
    //commandList_->SetPredication(nullptr, offset, D3D12_PREDICATION_OP_EQUAL_ZERO);
    //todo...
}

/* ----- Drawing ----- */

void D3D12RenderContext::Draw(unsigned int numVertices, unsigned int firstVertex)
{
    commandList_->DrawInstanced(numVertices, 1, firstVertex, 0);
}

void D3D12RenderContext::DrawIndexed(unsigned int numVertices, unsigned int firstIndex)
{
    commandList_->DrawIndexedInstanced(numVertices, 1, firstIndex, 0, 0);
}

void D3D12RenderContext::DrawIndexed(unsigned int numVertices, unsigned int firstIndex, int vertexOffset)
{
    commandList_->DrawIndexedInstanced(numVertices, 1, firstIndex, vertexOffset, 0);
}

void D3D12RenderContext::DrawInstanced(unsigned int numVertices, unsigned int firstVertex, unsigned int numInstances)
{
    commandList_->DrawInstanced(numVertices, numInstances, firstVertex, 0);
}

void D3D12RenderContext::DrawInstanced(unsigned int numVertices, unsigned int firstVertex, unsigned int numInstances, unsigned int instanceOffset)
{
    commandList_->DrawInstanced(numVertices, numInstances, firstVertex, instanceOffset);
}

void D3D12RenderContext::DrawIndexedInstanced(unsigned int numVertices, unsigned int numInstances, unsigned int firstIndex)
{
    commandList_->DrawIndexedInstanced(numVertices, numInstances, firstIndex, 0, 0);
}

void D3D12RenderContext::DrawIndexedInstanced(unsigned int numVertices, unsigned int numInstances, unsigned int firstIndex, int vertexOffset)
{
    commandList_->DrawIndexedInstanced(numVertices, numInstances, firstIndex, vertexOffset, 0);
}

void D3D12RenderContext::DrawIndexedInstanced(unsigned int numVertices, unsigned int numInstances, unsigned int firstIndex, int vertexOffset, unsigned int instanceOffset)
{
    commandList_->DrawIndexedInstanced(numVertices, numInstances, firstIndex, vertexOffset, instanceOffset);
}

/* ----- Compute ----- */

void D3D12RenderContext::DispatchCompute(const Gs::Vector3ui& threadGroupSize)
{
    //todo
}

/* ----- Misc ----- */

void D3D12RenderContext::SyncGPU()
{
    renderSystem_.SyncGPU(fenceValues_[currentFrame_]);
}


/*
 * ======= Private: =======
 */

void D3D12RenderContext::CreateWindowSizeDependentResources()
{
    /* Wait until all previous GPU work is complete */
    SyncGPU();

    /* Setup swap chain meta data */
    numFrames_ = static_cast<UINT>(desc_.videoMode.swapChainMode);

    /* Create swap chain for window handle */
    NativeHandle wndHandle;
    GetWindow().GetNativeHandle(&wndHandle);

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc;
    InitMemory(swapChainDesc);
    {
        swapChainDesc.Width                 = static_cast<UINT>(desc_.videoMode.resolution.x);
        swapChainDesc.Height                = static_cast<UINT>(desc_.videoMode.resolution.y);
        swapChainDesc.Format                = DXGI_FORMAT_B8G8R8A8_UNORM;
        swapChainDesc.Stereo                = false;
        swapChainDesc.SampleDesc.Count      = (desc_.antiAliasing.enabled ? desc_.antiAliasing.samples : 1);
        swapChainDesc.SampleDesc.Quality    = 0;
        swapChainDesc.BufferUsage           = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount           = numFrames_;
        swapChainDesc.SwapEffect            = DXGI_SWAP_EFFECT_FLIP_DISCARD;
        swapChainDesc.Flags                 = 0;
        swapChainDesc.Scaling               = DXGI_SCALING_NONE;
        swapChainDesc.AlphaMode             = DXGI_ALPHA_MODE_IGNORE;
    }
    auto swapChain = renderSystem_.CreateDXSwapChain(swapChainDesc, wndHandle.window);

    swapChain.As(swapChain_);

    /* Create RTV descriptor heap */
    D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc;
    InitMemory(descHeapDesc);
    {
        descHeapDesc.NumDescriptors = numFrames_;
        descHeapDesc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        descHeapDesc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    }
    rtvDescHeap_ = renderSystem_.CreateDXDescriptorHeap(descHeapDesc);
    rtvDescHeap_->SetName(L"render target view descriptor heap");

    rtvDescSize_ = renderSystem_.GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    /* Create render targets */
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvDescHandle(rtvDescHeap_->GetCPUDescriptorHandleForHeapStart());

    for (UINT i = 0; i < numFrames_; ++i)
    {
        /* Get render target resource from swap-chain buffer */
        auto hr = swapChain_->GetBuffer(i, IID_PPV_ARGS(&renderTargets_[i]));
        DXThrowIfFailed(hr, "failed to get D3D12 render target view " + std::to_string(i) + "/" + std::to_string(numFrames_) + " from swap chain");

        /* Create render target view (RTV) */
        /*D3D12_RENDER_TARGET_VIEW_DESC rtvDesc;
        rtvDesc.Format          = DXGI_FORMAT_R8G8B8A8_UNORM;
        rtvDesc.ViewDimension   = D3D12_RTV_DIMENSION_TEXTURE2DMS;*/

        renderSystem_.GetDevice()->CreateRenderTargetView(renderTargets_[i].Get(), nullptr, rtvDescHandle);

        rtvDescHandle.Offset(1, rtvDescSize_);
    }

    /* Update tracked fence values */
    for (UINT i = 0; i < numFrames_; ++i)
        fenceValues_[i] = fenceValues_[currentFrame_];

    /* Create command allocator and graphics command list */
    commandAlloc_ = renderSystem_.CreateDXCommandAllocator();
    commandList_ = renderSystem_.CreateDXCommandList(commandAlloc_.Get());

    /* Set initial render target view */
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvDescInit(rtvDescHeap_->GetCPUDescriptorHandleForHeapStart());
    commandList_->OMSetRenderTargets(1, &rtvDescInit, FALSE, nullptr);
}

void D3D12RenderContext::InitStateManager()
{
    /* Create state manager */
    stateMngr_ = MakeUnique<D3D12StateManager>(commandList_);

    /* Initialize states */
    auto resolution = desc_.videoMode.resolution;
    stateMngr_->SetViewports({ { 0.0f, 0.0f, static_cast<float>(resolution.x), static_cast<float>(resolution.y) } });
    stateMngr_->SetScissors({ { 0, 0, resolution.x, resolution.y } });
}

void D3D12RenderContext::MoveToNextFrame()
{
    /* Schedule signal command into the qeue */
    auto currentFenceValue = fenceValues_[currentFrame_];
    renderSystem_.SignalFenceValue(currentFenceValue);

    /* Advance frame index */
    currentFrame_ = swapChain_->GetCurrentBackBufferIndex();

    /* Check to see if the next frame is ready to start */
    renderSystem_.WaitForFenceValue(fenceValues_[currentFrame_]);

    /* Set fence value for next frame */
    fenceValues_[currentFrame_] = currentFenceValue + 1;
}

void D3D12RenderContext::SetBackBufferRTV()
{
    /* Set current back buffer as RTV */
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvDesc(rtvDescHeap_->GetCPUDescriptorHandleForHeapStart(), currentFrame_, rtvDescSize_);
    commandList_->OMSetRenderTargets(1, &rtvDesc, FALSE, nullptr);
}

void D3D12RenderContext::ExecuteCommandList()
{
    renderSystem_.CloseAndExecuteCommandList(commandList_.Get());
}

void D3D12RenderContext::SubmitConsistentStates()
{
    /* Submit all constistent states: viewports, scissors */
    stateMngr_->SubmitViewports();
    stateMngr_->SubmitScissors();
}


} // /namespace LLGL



// ================================================================================
