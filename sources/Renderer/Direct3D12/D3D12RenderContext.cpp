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

#include "Buffer/D3D12VertexBuffer.h"
#include "Buffer/D3D12IndexBuffer.h"
#include "Buffer/D3D12ConstantBuffer.h"
#include "Buffer/D3D12StorageBuffer.h"


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
    SetOrCreateWindow(window, desc_.videoMode, nullptr);
    CreateWindowSizeDependentResources();

    /* Initialize v-sync */
    SetVsync(desc_.vsync);
}

void D3D12RenderContext::Present()
{
    if (!commandList_)
        throw std::runtime_error("can not present framebuffer without D3D12 command allocator and/or command list");

    /* Resolve current render target if multi-sampling is used */
    if (desc_.multiSampling.enabled)
        ResolveRenderTarget();
    else
    {
        /* Indicate that the render target will now be used to present when the command list is done executing */
        TransitionRenderTarget(
            D3D12_RESOURCE_STATE_RENDER_TARGET,
            D3D12_RESOURCE_STATE_PRESENT
        );
    }

    /* Execute pending command list */
    renderSystem_.CloseAndExecuteCommandList(commandList_);

    /* Present swap-chain with vsync interval */
    auto hr = swapChain_->Present(swapChainInterval_, 0);
    DXThrowIfFailed(hr, "failed to present D3D12 swap-chain");

    /* Advance frame counter */
    MoveToNextFrame();

    /* Reset command allocator */
    hr = commandAllocs_[currentFrame_]->Reset();
    DXThrowIfFailed(hr, "failed to reset D3D12 command allocator");

    hr = commandList_->Reset(commandAllocs_[currentFrame_].Get(), nullptr);
    DXThrowIfFailed(hr, "failed to reset D3D12 command list");
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

/* --- Extended functions --- */

ID3D12Resource* D3D12RenderContext::GetCurrentRenderTarget()
{
    if (desc_.multiSampling.enabled)
        return renderTargetsMS_[currentFrame_].Get();
    else
        return renderTargets_[currentFrame_].Get();
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12RenderContext::GetCurrentRTVDescHandle() const
{
    return CD3DX12_CPU_DESCRIPTOR_HANDLE(
        rtvDescHeap_->GetCPUDescriptorHandleForHeapStart(),
        (desc_.multiSampling.enabled ? (numFrames_ + currentFrame_) : currentFrame_),
        rtvDescSize_
    );
}

void D3D12RenderContext::SetCommandList(ID3D12GraphicsCommandList* commandList)
{
    commandList_ = commandList;
}

void D3D12RenderContext::TransitionRenderTarget(D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter)
{
    /* Indicate a transition in the render-target usage and synchronize with the resource barrier */
    auto resourceBarrier = CD3DX12_RESOURCE_BARRIER::Transition(
        renderTargets_[currentFrame_].Get(),
        stateBefore, stateAfter
    );
    commandList_->ResourceBarrier(1, &resourceBarrier);
}

bool D3D12RenderContext::HasMultiSampling() const
{
    return desc_.multiSampling.enabled;
}


/*
 * ======= Private: =======
 */

void D3D12RenderContext::CreateWindowSizeDependentResources()
{
    auto device = renderSystem_.GetDevice();

    /* Wait until all previous GPU work is complete */
    //SyncGPU();

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
        swapChainDesc.SampleDesc.Count      = 1; // always 1 because D3D12 does not allow (directly) multi-sampled swap-chains
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
        descHeapDesc.NumDescriptors = (desc_.multiSampling.enabled ? numFrames_ * 2 : numFrames_);
        descHeapDesc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        descHeapDesc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    }
    rtvDescHeap_ = renderSystem_.CreateDXDescriptorHeap(descHeapDesc);
    rtvDescHeap_->SetName(L"render target view descriptor heap");

    rtvDescSize_ = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    /* Create render targets */
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvDescHandle(rtvDescHeap_->GetCPUDescriptorHandleForHeapStart());

    for (UINT i = 0; i < numFrames_; ++i)
    {
        /* Get render target resource from swap-chain buffer */
        auto hr = swapChain_->GetBuffer(i, IID_PPV_ARGS(renderTargets_[i].ReleaseAndGetAddressOf()));
        DXThrowIfFailed(hr, "failed to get D3D12 render target " + std::to_string(i) + "/" + std::to_string(numFrames_) + " from swap chain");

        /* Create render target view (RTV) */
        device->CreateRenderTargetView(renderTargets_[i].Get(), nullptr, rtvDescHandle);

        rtvDescHandle.Offset(1, rtvDescSize_);
    }

    if (desc_.multiSampling.enabled)
    {
        /* Create multi-sampled render targets */
        auto texture2DMSDesc = CD3DX12_RESOURCE_DESC::Tex2D(
            DXGI_FORMAT_R8G8B8A8_UNORM,
            swapChainDesc.Width,
            swapChainDesc.Height,
            1, // arraySize
            1, // mipLevels 
            std::max(1u, desc_.multiSampling.samples),
            0,
            D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
        );

        for (UINT i = 0; i < numFrames_; ++i)
        {
            /* Create render target resource */
            auto hr = device->CreateCommittedResource(
                &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
                D3D12_HEAP_FLAG_NONE,
                &texture2DMSDesc,
                D3D12_RESOURCE_STATE_COMMON,
                nullptr,
                IID_PPV_ARGS(renderTargetsMS_[i].ReleaseAndGetAddressOf())
            );
            DXThrowIfFailed(hr, "failed to create D3D12 multi-sampled render target " + std::to_string(i) + "/" + std::to_string(numFrames_) + " for swap chain");

            /* Create render target view (RTV) */
            device->CreateRenderTargetView(renderTargetsMS_[i].Get(), nullptr, rtvDescHandle);

            rtvDescHandle.Offset(1, rtvDescSize_);
        }
    }

    /* Update tracked fence values */
    for (UINT i = 0; i < numFrames_; ++i)
        fenceValues_[i] = fenceValues_[currentFrame_];

    /* Create command allocator and graphics command list */
    for (UINT i = 0; i < numFrames_; ++i)
        commandAllocs_[i] = renderSystem_.CreateDXCommandAllocator();
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

void D3D12RenderContext::ResolveRenderTarget()
{
    /* Prepare render-target for resolving */
    D3D12_RESOURCE_BARRIER resourceBarriersBefore[2] =
    {
        CD3DX12_RESOURCE_BARRIER::Transition(
            renderTargets_[currentFrame_].Get(),
            D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RESOLVE_DEST
        ),
        CD3DX12_RESOURCE_BARRIER::Transition(
            renderTargetsMS_[currentFrame_].Get(),
            D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_RESOLVE_SOURCE
        ),
    };
    commandList_->ResourceBarrier(2, resourceBarriersBefore);

    /* Resolve multi-sampled render targets */
    commandList_->ResolveSubresource(
        renderTargets_[currentFrame_].Get(), 0,
        renderTargetsMS_[currentFrame_].Get(), 0,
        DXGI_FORMAT_R8G8B8A8_UNORM
    );

    /* Prepare render-targets for presenting */
    D3D12_RESOURCE_BARRIER resourceBarriersAfter[2] =
    {
        CD3DX12_RESOURCE_BARRIER::Transition(
            renderTargets_[currentFrame_].Get(),
            D3D12_RESOURCE_STATE_RESOLVE_DEST, D3D12_RESOURCE_STATE_PRESENT
        ),
        CD3DX12_RESOURCE_BARRIER::Transition(
            renderTargetsMS_[currentFrame_].Get(),
            D3D12_RESOURCE_STATE_RESOLVE_SOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET
        ),
    };
    commandList_->ResourceBarrier(2, resourceBarriersAfter);
}


} // /namespace LLGL



// ================================================================================
