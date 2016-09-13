/*
 * D3D12RenderContext.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12RenderContext.h"
#include "D3D12RenderSystem.h"
#include "D3D12Assert.h"
#include "DXCore.h"
#include "DXTypes.h"
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
}

void D3D12RenderContext::Present()
{
    ExecuteGfxCommandList();

    /* Indicate that the render target will now be used to present when the command list is done executing */
    auto presentResourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
        renderTargets_[currentFrame_].Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PRESENT
    );

    gfxCommandList_->ResourceBarrier(1, &presentResourceBarrier);

    /* Present swap-chain with vsync interval */
    auto hr = swapChain_->Present(swapChainInterval_, 0);
    DXThrowIfFailed(hr, "failed to present D3D12 swap-chain");

    /* Advance frame counter */
    MoveToNextFrame();

    /* Reset command allocator */
    hr = commandAlloc_->Reset();
    DXThrowIfFailed(hr, "failed to reset D3D12 command allocator");

    hr = gfxCommandList_->Reset(commandAlloc_.Get(), nullptr);
    DXThrowIfFailed(hr, "failed to reset D3D12 graphics command list");
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
    if (desc_.vsync != vsyncDesc)
    {
        desc_.vsync = vsyncDesc;
        swapChainInterval_ = (vsyncDesc.enabled ? std::max(1u, std::min(vsyncDesc.interval, 4u)) : 0u);
    }
}

void D3D12RenderContext::SetViewports(const std::vector<Viewport>& viewports)
{
    gfxCommandList_->RSSetViewports(viewports.size(), reinterpret_cast<const D3D12_VIEWPORT*>(viewports.data()));
}

void D3D12RenderContext::SetScissors(const std::vector<Scissor>& scissors)
{
    //todo
}

void D3D12RenderContext::SetClearColor(const ColorRGBAf& color)
{
    clearColor_[0] = color.r;
    clearColor_[1] = color.g;
    clearColor_[2] = color.b;
    clearColor_[3] = color.a;
}

void D3D12RenderContext::SetClearDepth(float depth)
{
    clearDepth_ = depth;
}

void D3D12RenderContext::SetClearStencil(int stencil)
{
    clearStencil_ = stencil;
}

void D3D12RenderContext::ClearBuffers(long flags)
{
    /* Get RTV descriptor handle for current frame */
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
        rtvDescHeap_->GetCPUDescriptorHandleForHeapStart(), currentFrame_, rtvDescSize_
    );

    /* Clear color buffer */
    if ((flags & ClearBuffersFlags::Color) != 0)
        gfxCommandList_->ClearRenderTargetView(rtvHandle, clearColor_, 0, nullptr);
    
    /* Clear depth-stencil buffer */
    int rtvClearFlags = 0;

    if ((flags & ClearBuffersFlags::Depth) != 0)
        rtvClearFlags |= D3D12_CLEAR_FLAG_DEPTH;
    if ((flags & ClearBuffersFlags::Stencil) != 0)
        rtvClearFlags |= D3D12_CLEAR_FLAG_STENCIL;
        
    if (rtvClearFlags)
    {
        gfxCommandList_->ClearDepthStencilView(
            rtvHandle, static_cast<D3D12_CLEAR_FLAGS>(rtvClearFlags), clearDepth_, clearStencil_, 0, nullptr
        );
    }
}

void D3D12RenderContext::SetDrawMode(const DrawMode drawMode)
{
    gfxCommandList_->IASetPrimitiveTopology(DXTypes::Map(drawMode));
}

/* ----- Hardware Buffers ------ */

void D3D12RenderContext::BindVertexBuffer(VertexBuffer& vertexBuffer)
{
    auto& vertexBufferD3D = LLGL_CAST(D3D12VertexBuffer&, vertexBuffer);
    gfxCommandList_->IASetVertexBuffers(0, 1, &(vertexBufferD3D.GetView()));
}

void D3D12RenderContext::UnbindVertexBuffer()
{
    //todo
}

void D3D12RenderContext::BindIndexBuffer(IndexBuffer& indexBuffer)
{
    //todo
}

void D3D12RenderContext::UnbindIndexBuffer()
{
    //todo
}

void D3D12RenderContext::BindConstantBuffer(unsigned int index, ConstantBuffer& constantBuffer)
{
    //todo
}

void D3D12RenderContext::UnbindConstantBuffer(unsigned int index)
{
    //todo
}

void D3D12RenderContext::BindStorageBuffer(unsigned int index, StorageBuffer& storageBuffer)
{
    //todo
}

void D3D12RenderContext::UnbindStorageBuffer(unsigned int index)
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

void D3D12RenderContext::BindTexture(unsigned int layer, Texture& texture)
{
    //todo
}

void D3D12RenderContext::UnbindTexture(unsigned int layer)
{
    //todo
}

void D3D12RenderContext::GenerateMips(Texture& texture)
{
    //todo
}

/* ----- Sampler States ----- */

void D3D12RenderContext::BindSampler(unsigned int layer, Sampler& sampler)
{
    //todo
}

void D3D12RenderContext::UnbindSampler(unsigned int layer)
{
    //todo
}

/* ----- Render Targets ----- */

void D3D12RenderContext::BindRenderTarget(RenderTarget& renderTarget)
{
    //todo
}

void D3D12RenderContext::UnbindRenderTarget()
{
    //todo
}

/* ----- Pipeline States ----- */

void D3D12RenderContext::BindGraphicsPipeline(GraphicsPipeline& graphicsPipeline)
{
    auto& graphicsPipelineD3D = LLGL_CAST(D3D12GraphicsPipeline&, graphicsPipeline);
    gfxCommandList_->SetPipelineState(graphicsPipelineD3D.GetPipelineState());
}

void D3D12RenderContext::BindComputePipeline(ComputePipeline& computePipeline)
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

/* ----- Drawing ----- */

void D3D12RenderContext::Draw(unsigned int numVertices, unsigned int firstVertex)
{
    gfxCommandList_->DrawInstanced(numVertices, 1, firstVertex, 0);
}

void D3D12RenderContext::DrawIndexed(unsigned int numVertices, unsigned int firstIndex)
{
    gfxCommandList_->DrawIndexedInstanced(numVertices, 1, firstIndex, 0, 0);
}

void D3D12RenderContext::DrawIndexed(unsigned int numVertices, unsigned int firstIndex, int vertexOffset)
{
    gfxCommandList_->DrawIndexedInstanced(numVertices, 1, firstIndex, vertexOffset, 0);
}

void D3D12RenderContext::DrawInstanced(unsigned int numVertices, unsigned int firstVertex, unsigned int numInstances)
{
    gfxCommandList_->DrawInstanced(numVertices, numInstances, firstVertex, 0);
}

void D3D12RenderContext::DrawInstanced(unsigned int numVertices, unsigned int firstVertex, unsigned int numInstances, unsigned int instanceOffset)
{
    gfxCommandList_->DrawInstanced(numVertices, numInstances, firstVertex, instanceOffset);
}

void D3D12RenderContext::DrawIndexedInstanced(unsigned int numVertices, unsigned int numInstances, unsigned int firstIndex)
{
    gfxCommandList_->DrawIndexedInstanced(numVertices, numInstances, firstIndex, 0, 0);
}

void D3D12RenderContext::DrawIndexedInstanced(unsigned int numVertices, unsigned int numInstances, unsigned int firstIndex, int vertexOffset)
{
    gfxCommandList_->DrawIndexedInstanced(numVertices, numInstances, firstIndex, vertexOffset, 0);
}

void D3D12RenderContext::DrawIndexedInstanced(unsigned int numVertices, unsigned int numInstances, unsigned int firstIndex, int vertexOffset, unsigned int instanceOffset)
{
    gfxCommandList_->DrawIndexedInstanced(numVertices, numInstances, firstIndex, vertexOffset, instanceOffset);
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
    swapChain_ = renderSystem_.CreateDXSwapChain(swapChainDesc, wndHandle.window);

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
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvDesc(rtvDescHeap_->GetCPUDescriptorHandleForHeapStart());

    for (UINT i = 0; i < numFrames_; ++i)
    {
        /* Get render target resource from swap-chain buffer */
        auto hr = swapChain_->GetBuffer(i, IID_PPV_ARGS(&renderTargets_[i]));
        DXThrowIfFailed(hr, "failed to get D3D12 render target view " + std::to_string(i) + "/" + std::to_string(numFrames_) + " from swap chain");

        /* Create render target view (RTV) */
        renderSystem_.GetDevice()->CreateRenderTargetView(renderTargets_[i].Get(), nullptr, rtvDesc);

        rtvDesc.Offset(1, rtvDescSize_);
    }

    /* Update tracked fence values */
    for (UINT i = 0; i < numFrames_; ++i)
        fenceValues_[i] = fenceValues_[currentFrame_];

    /* Create command allocator and graphics command list */
    commandAlloc_ = renderSystem_.CreateDXCommandAllocator();
    gfxCommandList_ = renderSystem_.CreateDXGfxCommandList(commandAlloc_.Get());

    /* Set initial render target view */
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvDescInit(rtvDescHeap_->GetCPUDescriptorHandleForHeapStart());
    gfxCommandList_->OMSetRenderTargets(1, &rtvDescInit, FALSE, nullptr);
}

void D3D12RenderContext::SetupSwapChainInterval(const VsyncDescriptor& desc)
{
    swapChainInterval_ = (desc.enabled ? std::max(1u, std::min(desc.interval, 4u)) : 0);
}

void D3D12RenderContext::MoveToNextFrame()
{
    SyncGPU();
    currentFrame_ = (currentFrame_ + 1) % numFrames_;
}

void D3D12RenderContext::ExecuteGfxCommandList()
{
    /* Close graphics command list */
    auto hr = gfxCommandList_->Close();
    DXThrowIfFailed(hr, "failed to close D3D12 command list");

    /* Execute command list */
    ID3D12CommandList* commandList[] = { gfxCommandList_.Get() };
    renderSystem_.GetCommandQueue()->ExecuteCommandLists(1, commandList);
}


} // /namespace LLGL



// ================================================================================
