/*
 * D3D12RenderContext.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12RenderContext.h"
#include "D3D12RenderSystem.h"
#include "D3D12CommandContext.h"
#include "D3D12Types.h"
#include "../CheckedCast.h"
#include <LLGL/Platform/NativeHandle.h>
#include "../../Core/Helper.h"
#include "../DXCommon/DXTypes.h"
#include <algorithm>
#include "D3DX12/d3dx12.h"

#include "Buffer/D3D12VertexBuffer.h"
#include "Buffer/D3D12IndexBuffer.h"
#include "Buffer/D3D12ConstantBuffer.h"
#include "Buffer/D3D12StorageBuffer.h"


namespace LLGL
{


#ifdef LLGL_DEBUG
#   define LLGL_D3D12_SET_NAME(OBJ, NAME) (OBJ)->SetName(L"LLGL::D3D12RenderContext::" NAME)
#else
#   define LLGL_D3D12_SET_NAME(OBJ, NAME)
#endif

D3D12RenderContext::D3D12RenderContext(
    D3D12RenderSystem&              renderSystem,
    const RenderContextDescriptor&  desc,
    const std::shared_ptr<Surface>& surface) :
        RenderContext     { desc.videoMode, desc.vsync       },
        renderSystem_     { renderSystem                     },
        swapChainSamples_ { desc.multiSampling.SampleCount() }
{
    /* Setup surface for the render context */
    SetOrCreateSurface(surface, GetVideoMode(), nullptr);

    /* Create device resources and window dependent resource */
    CreateDeviceResources();
    CreateWindowSizeDependentResources(GetVideoMode());

    /* Initialize v-sync */
    OnSetVsync(desc.vsync);
}

D3D12RenderContext::~D3D12RenderContext()
{
    /* Ensure the GPU is no longer referencing resources that are about to be released */
    MoveToNextFrame();
}

void D3D12RenderContext::Present()
{
    /* Present swap-chain with vsync interval */
    auto hr = swapChain_->Present(swapChainInterval_, 0);
    DXThrowIfFailed(hr, "failed to present DXGI swap chain");

    /* Advance frame counter */
    MoveToNextFrame();
}

Format D3D12RenderContext::QueryColorFormat() const
{
    return DXTypes::Unmap(colorFormat_);
}

Format D3D12RenderContext::QueryDepthStencilFormat() const
{
    return DXTypes::Unmap(depthStencilFormat_);
}

const RenderPass* D3D12RenderContext::GetRenderPass() const
{
    return nullptr; // dummy
}

/* --- Extended functions --- */

D3D12Resource& D3D12RenderContext::GetCurrentColorBuffer()
{
    if (HasMultiSampling())
        return colorBuffersMS_[currentFrame_];
    else
        return colorBuffers_[currentFrame_];
}

void D3D12RenderContext::ResolveRenderTarget(D3D12CommandContext& commandContext)
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
            D3D12_RESOURCE_STATE_PRESENT
        );
    }
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12RenderContext::GetCPUDescriptorHandleForRTV() const
{
    return CD3DX12_CPU_DESCRIPTOR_HANDLE(
        rtvDescHeap_->GetCPUDescriptorHandleForHeapStart(),
        (HasMultiSampling() ? (numFrames_ + currentFrame_) : currentFrame_),
        rtvDescSize_
    );
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12RenderContext::GetCPUDescriptorHandleForDSV() const
{
    if (dsvDescHeap_)
        return dsvDescHeap_->GetCPUDescriptorHandleForHeapStart();
    else
        return {};
}

bool D3D12RenderContext::HasMultiSampling() const
{
    return (swapChainSamples_ > 1);
}

bool D3D12RenderContext::HasDepthBuffer() const
{
    return (depthStencilFormat_ != DXGI_FORMAT_UNKNOWN);
}

void D3D12RenderContext::SyncGPU()
{
    renderSystem_.SyncGPU(fenceValues_[currentFrame_]);
}


/*
 * ======= Private: =======
 */

bool D3D12RenderContext::OnSetVideoMode(const VideoModeDescriptor& videoModeDesc)
{
    const auto& prevVideoMode = GetVideoMode();

    /* Re-create resource that depend on the window size */
    if (prevVideoMode.resolution != videoModeDesc.resolution)
        CreateWindowSizeDependentResources(videoModeDesc);

    /* Switch fullscreen mode */
    if (prevVideoMode.fullscreen != videoModeDesc.fullscreen)
        swapChain_->SetFullscreenState(videoModeDesc.fullscreen ? TRUE : FALSE, nullptr);

    return true;
}

bool D3D12RenderContext::OnSetVsync(const VsyncDescriptor& vsyncDesc)
{
    swapChainInterval_ = (vsyncDesc.enabled ? std::max(1u, std::min(vsyncDesc.interval, 4u)) : 0u);
    return true;
}

void D3D12RenderContext::CreateWindowSizeDependentResources(const VideoModeDescriptor& videoModeDesc)
{
    /* Wait until all previous GPU work is complete */
    SyncGPU();

    /* Release previous window size dependent resources, and reset fence values to current value */
    for (UINT i = 0; i < numFrames_; ++i)
    {
        colorBuffers_[i].native.Reset();
        colorBuffersMS_[i].native.Reset();
        fenceValues_[i] = fenceValues_[currentFrame_];
    }

    depthStencil_.native.Reset();
    rtvDescHeap_.Reset();
    dsvDescHeap_.Reset();

    /* Setup swap chain meta data */
    numFrames_ = std::max(1u, std::min(videoModeDesc.swapChainSize, g_maxSwapChainSize));

    /* Get framebuffer size */
    auto framebufferWidth   = videoModeDesc.resolution.width;
    auto framebufferHeight  = videoModeDesc.resolution.height;

    if (swapChain_)
    {
        /* Resize swap chain */
        auto hr = swapChain_->ResizeBuffers(
            numFrames_,
            framebufferWidth,
            framebufferHeight,
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
        NativeHandle wndHandle;
        GetSurface().GetNativeHandle(&wndHandle);

        DXGI_SWAP_CHAIN_DESC1 swapChainDesc;
        {
            swapChainDesc.Width                 = framebufferWidth;
            swapChainDesc.Height                = framebufferHeight;
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

        swapChain.As(&swapChain_);
    }

    /* Create color buffer render target views (RTV) */
    CreateColorBufferRTVs(videoModeDesc);

    /* Update current back buffer index */
    currentFrame_ = swapChain_->GetCurrentBackBufferIndex();

    /* Create depth-stencil buffer (is used) */
    if (videoModeDesc.depthBits > 0 || videoModeDesc.stencilBits > 0)
        CreateDepthStencil(videoModeDesc);
    else
        depthStencilFormat_ = DXGI_FORMAT_UNKNOWN;
}

void D3D12RenderContext::CreateColorBufferRTVs(const VideoModeDescriptor& videoModeDesc)
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
    rtvDescHeap_ = renderSystem_.GetDevice().CreateDXDescriptorHeap(descHeapDesc);

    LLGL_D3D12_SET_NAME(rtvDescHeap_, L"rtvDescHeap");

    /* Create color buffers */
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvDescHandle(rtvDescHeap_->GetCPUDescriptorHandleForHeapStart());

    for (UINT i = 0; i < numFrames_; ++i)
    {
        /* Get render target resource from swap-chain buffer */
        auto hr = swapChain_->GetBuffer(i, IID_PPV_ARGS(colorBuffers_[i].native.ReleaseAndGetAddressOf()));
        DXThrowIfCreateFailed(hr, "ID3D12Resource", "for swap-chain color buffer");

        colorBuffers_[i].SetInitialState(D3D12_RESOURCE_STATE_PRESENT);

        /* Create render target view (RTV) */
        device->CreateRenderTargetView(colorBuffers_[i].native.Get(), nullptr, rtvDescHandle);

        #ifdef LLGL_DEBUG
        std::wstring name = L"LLGL::D3D12RenderContext::colorBuffer[" + std::to_wstring(i) + L"]";
        colorBuffers_[i].native->SetName(name.c_str());
        #endif

        rtvDescHandle.Offset(1, rtvDescSize_);
    }

    if (HasMultiSampling())
    {
        /* Create multi-sampled render targets */
        auto tex2DMSDesc = CD3DX12_RESOURCE_DESC::Tex2D(
            colorFormat_,
            videoModeDesc.resolution.width,
            videoModeDesc.resolution.height,
            1, // arraySize
            1, // mipLevels
            swapChainSamples_,
            0,
            D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
        );

        const FLOAT optimizedClearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

        for (UINT i = 0; i < numFrames_; ++i)
        {
            /* Create render target resource */
            auto hr = device->CreateCommittedResource(
                &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
                D3D12_HEAP_FLAG_NONE,
                &tex2DMSDesc,
                D3D12_RESOURCE_STATE_RENDER_TARGET,
                nullptr,//&CD3DX12_CLEAR_VALUE(colorFormat_, optimizedClearColor),
                IID_PPV_ARGS(colorBuffersMS_[i].native.ReleaseAndGetAddressOf())
            );
            DXThrowIfCreateFailed(hr, "ID3D12Resource", "for multi-sampled swap-chain");

            colorBuffersMS_[i].SetInitialState(D3D12_RESOURCE_STATE_RENDER_TARGET);

            /* Create render target view (RTV) */
            device->CreateRenderTargetView(colorBuffersMS_[i].native.Get(), nullptr, rtvDescHandle);

            rtvDescHandle.Offset(1, rtvDescSize_);
        }
    }
}

void D3D12RenderContext::CreateDepthStencil(const VideoModeDescriptor& videoModeDesc)
{
    auto device = renderSystem_.GetDXDevice();

    /* Pick-depth stencil format */
    depthStencilFormat_ = DXPickDepthStencilFormat(videoModeDesc.depthBits, videoModeDesc.stencilBits);

    /* Create DSV descriptor heap */
    D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc;
    {
        descHeapDesc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
        descHeapDesc.NumDescriptors = 1;
        descHeapDesc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        descHeapDesc.NodeMask       = 0;
    }
    dsvDescHeap_ = renderSystem_.GetDevice().CreateDXDescriptorHeap(descHeapDesc);

    LLGL_D3D12_SET_NAME(dsvDescHeap_, L"dsvDescHeap");

    /* Create depth-stencil buffer */
    auto tex2DDesc = CD3DX12_RESOURCE_DESC::Tex2D(
        depthStencilFormat_,
        videoModeDesc.resolution.width,
        videoModeDesc.resolution.height,
        1, // arraySize
        1, // mipLevels
        swapChainSamples_,
        0,
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

    LLGL_D3D12_SET_NAME(depthStencil_.native, L"depthStencil");

    /* Create depth-stencil view (DSV) */
    device->CreateDepthStencilView(depthStencil_.native.Get(), nullptr, dsvDescHeap_->GetCPUDescriptorHandleForHeapStart());
}

void D3D12RenderContext::CreateDeviceResources()
{
    /* Store size of RTV descriptor */
    rtvDescSize_ = renderSystem_.GetDXDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
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

#undef LLGL_D3D12_SET_NAME


} // /namespace LLGL



// ================================================================================
