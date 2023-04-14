/*
 * D3D12SwapChain.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D12SwapChain.h"
#include "D3D12RenderSystem.h"
#include "D3D12Types.h"
#include "D3D12ObjectUtils.h"
#include "Command/D3D12CommandContext.h"
#include "Command/D3D12CommandQueue.h"
#include "Buffer/D3D12Buffer.h"
#include "RenderState/D3D12DescriptorHeap.h"
#include "../CheckedCast.h"
#include "../../Core/CoreUtils.h"
#include "../DXCommon/DXTypes.h"

#include <LLGL/Platform/NativeHandle.h>
#include <LLGL/Log.h>
#include <LLGL/Misc/ForRange.h>
#include "D3DX12/d3dx12.h"
#include <algorithm>


namespace LLGL
{


D3D12SwapChain::D3D12SwapChain(
    D3D12RenderSystem&              renderSystem,
    const SwapChainDescriptor&      desc,
    const std::shared_ptr<Surface>& surface)
:
    SwapChain           { desc                                                       },
    renderSystem_       { renderSystem                                               },
    frameFence_         { renderSystem.GetDXDevice()                                 },
    depthStencilFormat_ { DXPickDepthStencilFormat(desc.depthBits, desc.stencilBits) },
    numFrames_          { std::max(1u, std::min(desc.swapBuffers, maxSwapBuffers))   }
{
    /* Store reference to command queue */
    commandQueue_ = LLGL_CAST(D3D12CommandQueue*, renderSystem_.GetCommandQueue());

    /* Setup surface for the swap-chain */
    SetOrCreateSurface(surface, desc.resolution, desc.fullscreen, nullptr);

    /* Create device resources and window dependent resource */
    QueryDeviceParameters(renderSystem.GetDevice(), desc.samples);
    CreateResolutionDependentResources(desc.resolution);

    /* Create default render pass */
    defaultRenderPass_.BuildAttachments(1, &colorFormat_, depthStencilFormat_, sampleDesc_);
}

D3D12SwapChain::~D3D12SwapChain()
{
    /* Ensure the GPU is no longer referencing resources that are about to be released */
    MoveToNextFrame();
}

void D3D12SwapChain::SetName(const char* name)
{
    D3D12SetObjectNameSubscript(rtvDescHeap_.Get(), name, ".RTV");
    D3D12SetObjectNameSubscript(dsvDescHeap_.Get(), name, ".DSV");

    std::string subscript;
    for (UINT i = 0; i < maxSwapBuffers; ++i)
    {
        subscript = (".BackBuffer" + std::to_string(i));
        D3D12SetObjectNameSubscript(colorBuffers_[i].Get(), name, subscript.c_str());

        subscript = (".BackBufferMS" + std::to_string(i));
        D3D12SetObjectNameSubscript(colorBuffersMS_[i].Get(), name, subscript.c_str());
    }

    D3D12SetObjectNameSubscript(depthStencil_.Get(), name, ".DS");
}

void D3D12SwapChain::Present()
{
    /* Present swap-chain with vsync interval */
    auto hr = swapChainDXGI_->Present(syncInterval_, 0);
    DXThrowIfFailed(hr, "failed to present DXGI swap chain");

    /* Advance frame counter */
    MoveToNextFrame();
}

std::uint32_t D3D12SwapChain::GetSamples() const
{
    return sampleDesc_.Count;
}

Format D3D12SwapChain::GetColorFormat() const
{
    return DXTypes::Unmap(colorFormat_);
}

Format D3D12SwapChain::GetDepthStencilFormat() const
{
    return DXTypes::Unmap(depthStencilFormat_);
}

const RenderPass* D3D12SwapChain::GetRenderPass() const
{
    return (&defaultRenderPass_);
}

bool D3D12SwapChain::SetVsyncInterval(std::uint32_t vsyncInterval)
{
    return SetPresentSyncInterval(vsyncInterval);
}

/* --- Extended functions --- */

D3D12Resource& D3D12SwapChain::GetCurrentColorBuffer()
{
    if (HasMultiSampling())
        return colorBuffersMS_[currentFrame_];
    else
        return colorBuffers_[currentFrame_];
}

void D3D12SwapChain::ResolveRenderTarget(D3D12CommandContext& commandContext)
{
    if (HasMultiSampling())
    {
        /* Resolve multi-sampled color buffer into presentable color buffer */
        commandContext.ResolveRenderTarget(
            colorBuffers_[currentFrame_],
            0,
            colorBuffersMS_[currentFrame_],
            0,
            colorFormat_
        );
    }
    else
    {
        /* Prepare color buffer for present */
        commandContext.TransitionResource(
            colorBuffers_[currentFrame_],
            D3D12_RESOURCE_STATE_PRESENT,
            true
        );
    }
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12SwapChain::GetCPUDescriptorHandleForRTV() const
{
    return CD3DX12_CPU_DESCRIPTOR_HANDLE(
        rtvDescHeap_->GetCPUDescriptorHandleForHeapStart(),
        (HasMultiSampling() ? (numFrames_ + currentFrame_) : currentFrame_),
        rtvDescSize_
    );
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12SwapChain::GetCPUDescriptorHandleForDSV() const
{
    if (dsvDescHeap_)
        return dsvDescHeap_->GetCPUDescriptorHandleForHeapStart();
    else
        return {};
}

bool D3D12SwapChain::HasMultiSampling() const
{
    return (sampleDesc_.Count > 1);
}

bool D3D12SwapChain::HasDepthBuffer() const
{
    return (depthStencilFormat_ != DXGI_FORMAT_UNKNOWN);
}

void D3D12SwapChain::SyncGPU()
{
    renderSystem_.SyncGPU();
}


/*
 * ======= Private: =======
 */

bool D3D12SwapChain::ResizeBuffersPrimary(const Extent2D& resolution)
{
    CreateResolutionDependentResources(resolution);
    return true;
}

bool D3D12SwapChain::SetPresentSyncInterval(UINT syncInterval)
{
    /* IDXGISwapChain::Present expects a sync interval in the range [0, 4] */
    if (syncInterval <= 4)
    {
        syncInterval_ = syncInterval;
        return true;
    }
    return false;
}

void D3D12SwapChain::QueryDeviceParameters(const D3D12Device& device, std::uint32_t samples)
{
    /* Find suitable sample descriptor */
    if (samples > 1)
        sampleDesc_ = device.FindSuitableSampleDesc(colorFormat_, samples);

    /* Store size of RTV descriptor */
    rtvDescSize_ = device.GetNative()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
}

void D3D12SwapChain::CreateResolutionDependentResources(const Extent2D& resolution)
{
    /* Wait until all previous GPU work is complete */
    SyncGPU();

    /* Release previous window size dependent resources, and reset fence values to current value */
    for_range(i, numFrames_)
    {
        colorBuffers_[i].native.Reset();
        colorBuffersMS_[i].native.Reset();
        frameFenceValues_[i] = frameFenceValues_[currentFrame_];
    }

    depthStencil_.native.Reset();
    rtvDescHeap_.Reset();
    dsvDescHeap_.Reset();

    /* Get framebuffer size */
    if (swapChainDXGI_)
    {
        /* Resize swap chain */
        auto hr = swapChainDXGI_->ResizeBuffers(
            numFrames_,
            resolution.width,
            resolution.height,
            colorFormat_,
            0
        );

        if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
        {
            /* Do not continue execution of this method, device resources will be destroyed and re-created */
            return;
        }
        else
            DXThrowIfFailed(hr, "failed to resize DXGI swap chain buffers");
    }
    else
    {
        /* Create swap chain for window handle */
        NativeHandle wndHandle = {};
        GetSurface().GetNativeHandle(&wndHandle, sizeof(wndHandle));

        DXGI_SWAP_CHAIN_DESC1 swapChainDesc;
        {
            swapChainDesc.Width                 = resolution.width;
            swapChainDesc.Height                = resolution.height;
            swapChainDesc.Format                = colorFormat_;
            swapChainDesc.Stereo                = FALSE;
            swapChainDesc.SampleDesc.Count      = 1; // always 1 because D3D12 does not allow (directly) multi-sampled swap-chains
            swapChainDesc.SampleDesc.Quality    = 0;
            swapChainDesc.BufferUsage           = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            swapChainDesc.BufferCount           = numFrames_;
            swapChainDesc.Scaling               = DXGI_SCALING_NONE;
            swapChainDesc.SwapEffect            = DXGI_SWAP_EFFECT_FLIP_DISCARD;
            swapChainDesc.AlphaMode             = DXGI_ALPHA_MODE_IGNORE;
            swapChainDesc.Flags                 = 0;
        }
        auto swapChain = renderSystem_.CreateDXSwapChain(swapChainDesc, wndHandle.window);

        swapChain.As(&swapChainDXGI_);
    }

    /* Create color buffer render target views (RTV) */
    CreateColorBufferRTVs(resolution);

    /* Update current back buffer index */
    currentFrame_ = swapChainDXGI_->GetCurrentBackBufferIndex();

    /* Create depth-stencil buffer (is used) */
    if (depthStencilFormat_ != DXGI_FORMAT_UNKNOWN)
        CreateDepthStencil(resolution);
}

void D3D12SwapChain::CreateColorBufferRTVs(const Extent2D& resolution)
{
    auto device = renderSystem_.GetDXDevice();

    /* Create RTV descriptor heap */
    D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc;
    {
        descHeapDesc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        descHeapDesc.NumDescriptors = (HasMultiSampling() ? numFrames_ * 2 : numFrames_);
        descHeapDesc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        descHeapDesc.NodeMask       = 0;
    }
    rtvDescHeap_ = D3D12DescriptorHeap::CreateNativeOrThrow(renderSystem_.GetDevice().GetNative(), descHeapDesc);

    /* Create color buffers */
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvDescHandle(rtvDescHeap_->GetCPUDescriptorHandleForHeapStart());

    for_range(i, numFrames_)
    {
        /* Get render target resource from swap-chain buffer */
        auto hr = swapChainDXGI_->GetBuffer(i, IID_PPV_ARGS(colorBuffers_[i].native.ReleaseAndGetAddressOf()));
        DXThrowIfCreateFailed(hr, "ID3D12Resource", "for swap-chain color buffer");

        colorBuffers_[i].SetInitialState(D3D12_RESOURCE_STATE_PRESENT);

        /* Create render target view (RTV) */
        device->CreateRenderTargetView(colorBuffers_[i].native.Get(), nullptr, rtvDescHandle);

        rtvDescHandle.Offset(1, rtvDescSize_);
    }

    if (HasMultiSampling())
    {
        /* Create multi-sampled render targets */
        auto tex2DMSDesc = CD3DX12_RESOURCE_DESC::Tex2D(
            colorFormat_,
            resolution.width,
            resolution.height,
            1, // arraySize
            1, // mipLevels
            sampleDesc_.Count,
            sampleDesc_.Quality,
            D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
        );

        for (UINT i = 0; i < numFrames_; ++i)
        {
            /* Create render target resource */
            auto hr = device->CreateCommittedResource(
                &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
                D3D12_HEAP_FLAG_NONE,
                &tex2DMSDesc,
                D3D12_RESOURCE_STATE_RENDER_TARGET,
                nullptr,
                IID_PPV_ARGS(colorBuffersMS_[i].native.ReleaseAndGetAddressOf())
            );
            DXThrowIfCreateFailed(hr, "ID3D12Resource", "for swap-chain multi-sampled color buffer");

            colorBuffersMS_[i].SetInitialState(D3D12_RESOURCE_STATE_RENDER_TARGET);

            /* Create render target view (RTV) */
            device->CreateRenderTargetView(colorBuffersMS_[i].native.Get(), nullptr, rtvDescHandle);

            rtvDescHandle.Offset(1, rtvDescSize_);
        }
    }
}

void D3D12SwapChain::CreateDepthStencil(const Extent2D& resolution)
{
    auto device = renderSystem_.GetDXDevice();

    /* Create DSV descriptor heap */
    D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc;
    {
        descHeapDesc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        descHeapDesc.NumDescriptors = 1;
        descHeapDesc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        descHeapDesc.NodeMask       = 0;
    }
    dsvDescHeap_ = D3D12DescriptorHeap::CreateNativeOrThrow(renderSystem_.GetDevice().GetNative(), descHeapDesc);

    /* Create depth-stencil buffer */
    auto tex2DDesc = CD3DX12_RESOURCE_DESC::Tex2D(
        depthStencilFormat_,
        resolution.width,
        resolution.height,
        1, // arraySize
        1, // mipLevels
        sampleDesc_.Count,
        sampleDesc_.Quality,
        (D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL | D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE)
    );

    auto hr = device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &tex2DDesc,
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &CD3DX12_CLEAR_VALUE(depthStencilFormat_, 1.0f, 0),
        IID_PPV_ARGS(depthStencil_.native.ReleaseAndGetAddressOf())
    );
    DXThrowIfCreateFailed(hr, "ID3D12Resource", "for swap-chain depth-stencil buffer");

    /* Create depth-stencil view (DSV) */
    device->CreateDepthStencilView(depthStencil_.native.Get(), nullptr, dsvDescHeap_->GetCPUDescriptorHandleForHeapStart());
}

void D3D12SwapChain::MoveToNextFrame()
{
    /* Schedule signal command into the queue */
    const UINT64 currentFenceValue = frameFenceValues_[currentFrame_];
    commandQueue_->SignalFence(frameFence_.Get(), currentFenceValue);

    /* Advance frame index */
    currentFrame_ = swapChainDXGI_->GetCurrentBackBufferIndex();

    /* Wait until the fence value of the next frame is signaled, so we know the next frame is ready to start */
    frameFence_.WaitForHigherSignal(frameFenceValues_[currentFrame_]);
    frameFenceValues_[currentFrame_] = currentFenceValue + 1;
}


} // /namespace LLGL



// ================================================================================
