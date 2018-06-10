/*
 * D3D12RenderContext.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
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


#ifdef LLGL_DEBUG
#   define LLGL_D3D12_SET_NAME(OBJ, NAME) (OBJ)->SetName(L"LLGL::D3D12RenderContext::" NAME)
#else
#   define LLGL_D3D12_SET_NAME(OBJ, NAME)
#endif

D3D12RenderContext::D3D12RenderContext(
    D3D12RenderSystem& renderSystem,
    RenderContextDescriptor desc,
    const std::shared_ptr<Surface>& surface) :
        RenderContext     { desc.videoMode, desc.vsync       },
        renderSystem_     { renderSystem                     },
        swapChainSamples_ { desc.multiSampling.SampleCount() }
{
    #if 1 //TODO: multi-sampling currently not supported!
    swapChainSamples_ = 1;
    #endif

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
    /* Get command list from command buffer object */
    if (!commandBuffer_)
        throw std::runtime_error("cannot present framebuffer without D3D12 command allocator and/or command list");

    auto commandList = commandBuffer_->GetCommandList();

    if (HasMultiSampling())
    {
        /* Blit multi-sampled texture into back-buffer */
        ResolveRenderTarget(commandList);
    }
    else
    {
        /* Indicate that the render target will now be used to present when the command list is done executing */
        TransitionRenderTarget(D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    }

    /* Execute pending command list */
    renderSystem_.CloseAndExecuteCommandList(commandList);

    /* Present swap-chain with vsync interval */
    auto hr = swapChain_->Present(swapChainInterval_, 0);
    DXThrowIfFailed(hr, "failed to present DXGI swap chain");

    /* Advance frame counter */
    MoveToNextFrame();

    /* Reset command allocator and command list*/
    auto commandAlloc = commandAllocs_[currentFrame_].Get();

    hr = commandAlloc->Reset();
    DXThrowIfFailed(hr, "failed to reset D3D12 command allocator");

    commandBuffer_->ResetCommandList(commandAlloc, nullptr);
}

/* --- Extended functions --- */

ID3D12Resource* D3D12RenderContext::GetCurrentColorBuffer()
{
    if (HasMultiSampling())
        return colorBuffersMS_[currentFrame_].Get();
    else
        return colorBuffers_[currentFrame_].Get();
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12RenderContext::GetCPUDescriptorHandleForCurrentRTV() const
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

void D3D12RenderContext::SetCommandBuffer(D3D12CommandBuffer* commandBuffer)
{
    commandBuffer_ = commandBuffer;
}

void D3D12RenderContext::TransitionRenderTarget(D3D12_RESOURCE_STATES stateBefore, D3D12_RESOURCE_STATES stateAfter)
{
    /* Indicate a transition in the render-target usage and synchronize with the resource barrier */
    commandBuffer_->GetCommandList()->ResourceBarrier(
        1, &CD3DX12_RESOURCE_BARRIER::Transition(colorBuffers_[currentFrame_].Get(), stateBefore, stateAfter)
    );
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
        colorBuffers_[i].Reset();
        colorBuffersMS_[i].Reset();
        fenceValues_[i] = fenceValues_[currentFrame_];
    }

    depthStencil_.Reset();
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
            colorBufferFormat_,
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
            swapChainDesc.Format                = colorBufferFormat_;
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
    auto device = renderSystem_.GetDevice();

    /* Create RTV descriptor heap */
    D3D12_DESCRIPTOR_HEAP_DESC descHeapDesc;
    {
        descHeapDesc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        descHeapDesc.NumDescriptors = (HasMultiSampling() ? numFrames_ * 2 : numFrames_);
        descHeapDesc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        descHeapDesc.NodeMask       = 0;
    }
    rtvDescHeap_ = renderSystem_.CreateDXDescriptorHeap(descHeapDesc);

    LLGL_D3D12_SET_NAME(rtvDescHeap_, L"rtvDescHeap");

    /* Create color buffers */
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvDescHandle(rtvDescHeap_->GetCPUDescriptorHandleForHeapStart());

    for (UINT i = 0; i < numFrames_; ++i)
    {
        /* Get render target resource from swap-chain buffer */
        auto hr = swapChain_->GetBuffer(i, IID_PPV_ARGS(colorBuffers_[i].ReleaseAndGetAddressOf()));

        if (FAILED(hr))
        {
            std::string info = "failed to get D3D12 render target " + std::to_string(i) + "/" + std::to_string(numFrames_) + " from swap chain";
            DXThrowIfFailed(hr, info.c_str());
        }

        /* Create render target view (RTV) */
        device->CreateRenderTargetView(colorBuffers_[i].Get(), nullptr, rtvDescHandle);

        #ifdef LLGL_DEBUG
        std::wstring name = L"LLGL::D3D12RenderContext::colorBuffer" + std::to_wstring(i);
        colorBuffers_[i]->SetName(name.c_str());
        #endif

        rtvDescHandle.Offset(1, rtvDescSize_);
    }

    if (HasMultiSampling())
    {
        /* Create multi-sampled render targets */
        auto tex2DMSDesc = CD3DX12_RESOURCE_DESC::Tex2D(
            colorBufferFormat_,
            videoModeDesc.resolution.width,
            videoModeDesc.resolution.height,
            1, // arraySize
            1, // mipLevels
            swapChainSamples_,
            0,
            D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
        );

        for (UINT i = 0; i < numFrames_; ++i)
        {
            /* Create render target resource */
            auto hr = device->CreateCommittedResource(
                &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
                D3D12_HEAP_FLAG_NONE,
                &tex2DMSDesc,
                D3D12_RESOURCE_STATE_COMMON,
                nullptr,
                IID_PPV_ARGS(colorBuffersMS_[i].ReleaseAndGetAddressOf())
            );

            if (FAILED(hr))
            {
                std::string info = "failed to create D3D12 multi-sampled render target " + std::to_string(i) + "/" + std::to_string(numFrames_) + " for swap chain";
                DXThrowIfFailed(hr, info.c_str());
            }

            /* Create render target view (RTV) */
            device->CreateRenderTargetView(colorBuffersMS_[i].Get(), nullptr, rtvDescHandle);

            rtvDescHandle.Offset(1, rtvDescSize_);
        }
    }
}

void D3D12RenderContext::CreateDepthStencil(const VideoModeDescriptor& videoModeDesc)
{
    auto device = renderSystem_.GetDevice();

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
    dsvDescHeap_ = renderSystem_.CreateDXDescriptorHeap(descHeapDesc);

    LLGL_D3D12_SET_NAME(dsvDescHeap_, L"dsvDescHeap");

    /* Speciy DSV clear value that is most optimized */
    D3D12_CLEAR_VALUE optimizedClearValue;
    optimizedClearValue.Format               = depthStencilFormat_;
    optimizedClearValue.DepthStencil.Depth   = 1.0f;
    optimizedClearValue.DepthStencil.Stencil = 0;

    /* Create depth-stencil buffer */
    auto tex2DDesc = CD3DX12_RESOURCE_DESC::Tex2D(
        depthStencilFormat_,
        videoModeDesc.resolution.width,
        videoModeDesc.resolution.height,
        1, // arraySize
        1, // mipLevels
        1,
        0,
        (D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL | D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE)
    );

    auto hr = device->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
        D3D12_HEAP_FLAG_NONE,
        &tex2DDesc,
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &optimizedClearValue,
        IID_PPV_ARGS(depthStencil_.ReleaseAndGetAddressOf())
    );

    LLGL_D3D12_SET_NAME(depthStencil_, L"depthStencil");

    /* Create depth-stencil view (DSV) */
    device->CreateDepthStencilView(depthStencil_.Get(), nullptr, dsvDescHeap_->GetCPUDescriptorHandleForHeapStart());
}

void D3D12RenderContext::CreateDeviceResources()
{
    /* Store size of RTV descriptor */
    rtvDescSize_ = renderSystem_.GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    /* Create command allocators */
    for (UINT i = 0; i < g_maxSwapChainSize; ++i)
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

void D3D12RenderContext::ResolveRenderTarget(ID3D12GraphicsCommandList* commandList)
{
    /* Prepare render-target for resolving */
    D3D12_RESOURCE_BARRIER resourceBarriersBefore[2] =
    {
        CD3DX12_RESOURCE_BARRIER::Transition(
            colorBuffers_[currentFrame_].Get(),
            D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RESOLVE_DEST
        ),
        CD3DX12_RESOURCE_BARRIER::Transition(
            colorBuffersMS_[currentFrame_].Get(),
            D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_RESOLVE_SOURCE
        ),
    };
    commandList->ResourceBarrier(2, resourceBarriersBefore);

    /* Resolve multi-sampled render targets */
    commandList->ResolveSubresource(
        colorBuffers_[currentFrame_].Get(), 0,
        colorBuffersMS_[currentFrame_].Get(), 0,
        colorBufferFormat_
    );

    /* Prepare render-targets for presenting */
    D3D12_RESOURCE_BARRIER resourceBarriersAfter[2] =
    {
        CD3DX12_RESOURCE_BARRIER::Transition(
            colorBuffers_[currentFrame_].Get(),
            D3D12_RESOURCE_STATE_RESOLVE_DEST, D3D12_RESOURCE_STATE_PRESENT
        ),
        CD3DX12_RESOURCE_BARRIER::Transition(
            colorBuffersMS_[currentFrame_].Get(),
            D3D12_RESOURCE_STATE_RESOLVE_SOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET
        ),
    };
    commandList->ResourceBarrier(2, resourceBarriersAfter);
}

#undef LLGL_D3D12_SET_NAME


} // /namespace LLGL



// ================================================================================
