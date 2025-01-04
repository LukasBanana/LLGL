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
#include <LLGL/Utils/ForRange.h>
#include "D3DX12/d3dx12.h"
#include <algorithm>


namespace LLGL
{


constexpr UINT D3D12SwapChain::maxNumColorBuffers;
constexpr UINT D3D12SwapChain::numDebugNames;

D3D12SwapChain::D3D12SwapChain(
    D3D12RenderSystem&              renderSystem,
    const SwapChainDescriptor&      desc,
    const std::shared_ptr<Surface>& surface)
:
    SwapChain           { desc                                                            },
    renderSystem_       { renderSystem                                                    },
    depthStencilFormat_ { DXPickDepthStencilFormat(desc.depthBits, desc.stencilBits)      },
    frameFence_         { renderSystem.GetDXDevice()                                      },
    numColorBuffers_    { Clamp(desc.swapBuffers, 1u, D3D12SwapChain::maxNumColorBuffers) },
    tearingSupported_   { renderSystem.IsTearingSupported()                               }
{
    /* Store reference to command queue */
    commandQueue_ = LLGL_CAST(D3D12CommandQueue*, renderSystem_.GetCommandQueue());

    /* Setup surface for the swap-chain */
    SetOrCreateSurface(surface, SwapChain::BuildDefaultSurfaceTitle(renderSystem.GetRendererInfo()), desc.resolution, desc.fullscreen);

    /* Create device resources and window dependent resource */
    CreateDescriptorHeaps(renderSystem.GetDevice(), desc.samples);
    CreateResolutionDependentResources(GetResolution());

    /* Create default render pass */
    defaultRenderPass_.BuildAttachments(1, &colorFormat_, depthStencilFormat_, sampleDesc_);

    if (desc.debugName != nullptr)
        SetDebugName(desc.debugName);

    /* Show default surface */
    if (!surface)
        ShowSurface();
}

D3D12SwapChain::~D3D12SwapChain()
{
    /* Ensure the GPU is no longer referencing resources that are about to be released */
    MoveToNextFrame();
}

void D3D12SwapChain::SetDebugName(const char* name)
{
    if (name != nullptr)
    {
        D3D12SetObjectNameSubscript(rtvDescHeap_.Get(), name, ".RTV");
        D3D12SetObjectNameSubscript(dsvDescHeap_.Get(), name, ".DSV");

        std::string subscript;
        for_range(i, D3D12SwapChain::maxNumColorBuffers)
        {
            subscript = (".BackBuffer" + std::to_string(i));
            D3D12SetObjectNameSubscript(colorBuffers_[i].Get(), name, subscript.c_str());

            subscript = (".BackBufferMS" + std::to_string(i));
            D3D12SetObjectNameSubscript(colorBuffersMS_[i].Get(), name, subscript.c_str());
        }

        D3D12SetObjectNameSubscript(depthStencil_.Get(), name, ".DS");

        hasDebugName_ = true;
    }
    else
    {
        D3D12SetObjectName(rtvDescHeap_.Get(), nullptr);
        D3D12SetObjectName(dsvDescHeap_.Get(), nullptr);

        for_range(i, D3D12SwapChain::maxNumColorBuffers)
        {
            D3D12SetObjectName(colorBuffers_[i].Get(), nullptr);
            D3D12SetObjectName(colorBuffersMS_[i].Get(), nullptr);
        }

        D3D12SetObjectName(depthStencil_.Get(), nullptr);

        hasDebugName_ = false;
    }
}

bool D3D12SwapChain::IsPresentable() const
{
    return true; // dummy
}

void D3D12SwapChain::Present()
{
    /* Present swap-chain with vsync interval */
    const bool tearingEnabled   = (tearingSupported_ && windowedMode_ && syncInterval_ == 0);
    const UINT presentFlags     = (tearingEnabled ? DXGI_PRESENT_ALLOW_TEARING : 0u);

    HRESULT hr = S_OK;
    if (isPresentationDirty_)
    {
        /* Don't perform vsync when the back buffer has been resized to allow a smooth window resizing */
        isPresentationDirty_ = false;
        hr = swapChainDXGI_->Present(0, presentFlags);
    }
    else
        hr = swapChainDXGI_->Present(syncInterval_, presentFlags);

    DXThrowIfFailed(hr, "failed to present DXGI swap chain");

    /* Advance frame counter */
    MoveToNextFrame();
}

std::uint32_t D3D12SwapChain::GetCurrentSwapIndex() const
{
    return currentColorBuffer_;
}

std::uint32_t D3D12SwapChain::GetNumSwapBuffers() const
{
    return numColorBuffers_;
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

UINT D3D12SwapChain::TranslateSwapIndex(std::uint32_t swapBufferIndex) const
{
    if (swapBufferIndex == LLGL_CURRENT_SWAP_INDEX)
        return currentColorBuffer_;
    else
        return std::min(swapBufferIndex, numColorBuffers_ - 1);
}

D3D12Resource& D3D12SwapChain::GetCurrentColorBuffer(UINT colorBuffer)
{
    if (HasMultiSampling())
        return colorBuffersMS_[colorBuffer];
    else
        return colorBuffers_[colorBuffer];
}

void D3D12SwapChain::ResolveSubresources(D3D12CommandContext& commandContext, UINT colorBuffer)
{
    if (HasMultiSampling())
    {
        /* Resolve multi-sampled color buffer into presentable color buffer */
        commandContext.ResolveSubresource(
            colorBuffers_[colorBuffer],
            0,
            colorBuffersMS_[colorBuffer],
            0,
            colorFormat_
        );
    }
    else
    {
        /* Prepare color buffer for present */
        commandContext.TransitionResource(
            colorBuffers_[colorBuffer],
            D3D12_RESOURCE_STATE_PRESENT,
            true
        );
    }
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12SwapChain::GetCPUDescriptorHandleForRTV(UINT colorBuffer) const
{
    D3D12_CPU_DESCRIPTOR_HANDLE rtvDescHandle = rtvDescHeap_->GetCPUDescriptorHandleForHeapStart();
    rtvDescHandle.ptr += colorBuffer * rtvDescSize_;
    return rtvDescHandle;
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

static bool IsD3D12BoxCoveringWholeResource(UINT width, UINT height, const D3D12_BOX& box)
{
    return
    (
        box.left    == 0        &&
        box.top     == 0        &&
        box.front   == 0        &&
        box.right   == width    &&
        box.bottom  == height   &&
        box.back    == 1
    );
}

static void D3D12CopyFramebufferSubresourceRegion(
    D3D12CommandContext&    context,
    D3D12Resource&          dstResource,
    UINT                    dstSubresource,
    UINT                    dstX,
    UINT                    dstY,
    UINT                    dstZ,
    D3D12Resource&          srcResource,
    D3D12Resource*          intermediateResource,
    const D3D12_BOX&        srcBox)
{
    ID3D12GraphicsCommandList* commandList = context.GetCommandList();

    /* Check if whole resource must be copied with an intermediate texture */
    D3D12_RESOURCE_DESC srcResourceDesc = srcResource.Get()->GetDesc();
    const bool isSrcMultisampled = (srcResourceDesc.SampleDesc.Count > 1);

    if (isSrcMultisampled)
    {
        D3D12_RESOURCE_DESC dstResourceDesc = dstResource.Get()->GetDesc();

        const bool isSameDimension = (dstResourceDesc.Dimension == srcResourceDesc.Dimension);
        const bool isDstOffsetZero = (dstX == 0 && dstY == 0 && dstZ == 0);
        const bool isWholeResource = IsD3D12BoxCoveringWholeResource(static_cast<UINT>(srcResourceDesc.Width), srcResourceDesc.Height, srcBox);

        if (isSameDimension && isWholeResource && isDstOffsetZero)
        {
            if (dstResourceDesc.SampleDesc.Count == srcResourceDesc.SampleDesc.Count)
            {
                /* Copy multi-sampled texture directly into destination multi-sampled texture */
                context.CopyTextureRegion(dstResource, dstSubresource, 0, 0, 0, srcResource, 0, nullptr);
            }
            else
            {
                /* Resolve multi-sampled texture directly into destination texture */
                context.ResolveSubresource(dstResource, dstSubresource, srcResource, 0, dstResourceDesc.Format);
            }
        }
        else
        {
            /* Resolve into intermdiate resource and then copy its region into the destination resource */
            LLGL_ASSERT_PTR(intermediateResource);
            context.ResolveSubresource(*intermediateResource, 0, srcResource, 0, srcResourceDesc.Format);
            context.CopyTextureRegion(dstResource, dstSubresource, dstX, dstY, dstZ, *intermediateResource, 0, &srcBox);
        }
    }
    else
    {
        /* Copy subresource region directly */
        context.CopyTextureRegion(dstResource, dstSubresource, dstX, dstY, dstZ, srcResource, 0, &srcBox);
    }
}

HRESULT D3D12SwapChain::CopySubresourceRegion(
    D3D12CommandContext&    context,
    D3D12Resource&          dstResource,
    UINT                    dstSubresource,
    UINT                    dstX,
    UINT                    dstY,
    UINT                    dstZ,
    UINT                    srcColorBuffer,
    const D3D12_BOX&        srcBox,
    DXGI_FORMAT             format)
{
    const bool isDepthStencil = DXTypes::IsDepthStencilDXGIFormat(format);
    if (isDepthStencil)
    {
        if (depthStencil_.Get() == nullptr)
            return E_FAIL;
        if (DXTypes::ToDXGIFormatTypeless(depthStencilFormat_) != DXTypes::ToDXGIFormatTypeless(format))
            return E_INVALIDARG;
        if (HasMultiSampling())
        {
            LLGL_TRAP_NOT_IMPLEMENTED("multi-sampled depth-stencil"); //TODO
        }
        else
            D3D12CopyFramebufferSubresourceRegion(context, dstResource, dstSubresource, dstX, dstY, dstZ, depthStencil_, nullptr, srcBox);
    }
    else
    {
        if (srcColorBuffer >= D3D12SwapChain::maxNumColorBuffers)
            return E_INVALIDARG;
        if (colorBuffers_[srcColorBuffer].Get() == nullptr)
            return E_FAIL;
        if (HasMultiSampling())
        {
            D3D12Resource& srcResource          = colorBuffersMS_[srcColorBuffer];
            D3D12Resource& intermediateResource = colorBuffers_[srcColorBuffer];
            D3D12CopyFramebufferSubresourceRegion(context, dstResource, dstSubresource, dstX, dstY, dstZ, srcResource, &intermediateResource, srcBox);
        }
        else
        {
            D3D12Resource& srcResource = colorBuffers_[srcColorBuffer];
            D3D12CopyFramebufferSubresourceRegion(context, dstResource, dstSubresource, dstX, dstY, dstZ, srcResource, nullptr, srcBox);
        }
    }

    return S_OK;
}


/*
 * ======= Private: =======
 */

bool D3D12SwapChain::ResizeBuffersPrimary(const Extent2D& resolution)
{
    CreateResolutionDependentResources(resolution);

    /* Mark presentation as dirty to avoid vsync on the next presentation; This allows a smooth window resizing like in other backends */
    isPresentationDirty_ = true;

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

void D3D12SwapChain::CreateDescriptorHeaps(const D3D12Device& device, std::uint32_t samples)
{
    /* Find suitable sample descriptor */
    if (samples > 1)
        sampleDesc_ = device.FindSuitableSampleDesc(colorFormat_, samples);

    /* Store size of RTV descriptor */
    rtvDescSize_ = device.GetNative()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    /* Create RTV descriptor heap */
    D3D12_DESCRIPTOR_HEAP_DESC rtvDescHeapDesc;
    {
        rtvDescHeapDesc.Type            = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
        rtvDescHeapDesc.NumDescriptors  = numColorBuffers_;
        rtvDescHeapDesc.Flags           = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
        rtvDescHeapDesc.NodeMask        = 0;
    }
    rtvDescHeap_ = D3D12DescriptorHeap::CreateNativeOrThrow(device.GetNative(), rtvDescHeapDesc);

    /* Create DSV descriptor heap */
    if (HasDepthBuffer())
    {
        D3D12_DESCRIPTOR_HEAP_DESC dsvDescHeapDesc;
        {
            dsvDescHeapDesc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
            dsvDescHeapDesc.NumDescriptors = 1;
            dsvDescHeapDesc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
            dsvDescHeapDesc.NodeMask       = 0;
        }
        dsvDescHeap_ = D3D12DescriptorHeap::CreateNativeOrThrow(device.GetNative(), dsvDescHeapDesc);
    }
}

HRESULT D3D12SwapChain::CreateResolutionDependentResources(const Extent2D& resolution)
{
    ID3D12Device* device = renderSystem_.GetDXDevice();

    /* Wait until all previous GPU work is complete */
    renderSystem_.SyncGPU();

    /* Store current debug names */
    std::string debugNames[D3D12SwapChain::numDebugNames];
    if (hasDebugName_)
        StoreDebugNames(debugNames);

    /* Release previous window size dependent resources, and reset fence values to current value */
    for_range(i, numColorBuffers_)
    {
        colorBuffers_[i].native.Reset();
        colorBuffersMS_[i].native.Reset();
        frameFenceValues_[i] = frameFenceValues_[currentColorBuffer_];
    }

    depthStencil_.native.Reset();

    /* Get framebuffer size */
    if (swapChainDXGI_)
    {
        DXGI_SWAP_CHAIN_DESC desc;
        swapChainDXGI_->GetDesc(&desc);

        /* Resize swap chain */
        HRESULT hr = swapChainDXGI_->ResizeBuffers(
            numColorBuffers_,
            resolution.width,
            resolution.height,
            colorFormat_,
            desc.Flags
        );

        if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
        {
            /* Do not continue execution of this method, device resources will be destroyed and re-created */
            return hr;
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
            swapChainDesc.BufferCount           = numColorBuffers_;
            swapChainDesc.Scaling               = DXGI_SCALING_NONE;
            swapChainDesc.SwapEffect            = DXGI_SWAP_EFFECT_FLIP_DISCARD;
            swapChainDesc.AlphaMode             = DXGI_ALPHA_MODE_IGNORE;
            swapChainDesc.Flags                 = (tearingSupported_ ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0u);
        }
        auto swapChain = renderSystem_.CreateDXSwapChain(swapChainDesc, &wndHandle, sizeof(wndHandle));

        swapChain.As(&swapChainDXGI_);
    }

    /* Store windowed mode for tearing support */
    windowedMode_ = !DXGetFullscreenState(swapChainDXGI_.Get());

    /* Create color buffer render target views (RTV) */
    CreateColorBufferRTVs(device, resolution);

    /* Update current back buffer index */
    currentColorBuffer_ = swapChainDXGI_->GetCurrentBackBufferIndex();

    /* Create depth-stencil buffer (is used) */
    if (HasDepthBuffer())
        CreateDepthStencil(device, resolution);

    /* Restore debug names with new swap-chain buffers */
    if (hasDebugName_)
        RestoreDebugNames(debugNames);

    return S_OK;
}

void D3D12SwapChain::CreateColorBufferRTVs(ID3D12Device* device, const Extent2D& resolution)
{
    /* Create color buffers */
    for_range(i, numColorBuffers_)
    {
        /* Get render target resource from swap-chain buffer */
        HRESULT hr = swapChainDXGI_->GetBuffer(i, IID_PPV_ARGS(colorBuffers_[i].native.ReleaseAndGetAddressOf()));
        DXThrowIfCreateFailed(hr, "ID3D12Resource", "for swap-chain color buffer");

        colorBuffers_[i].SetInitialState(D3D12_RESOURCE_STATE_PRESENT);
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

        for_range(i, numColorBuffers_)
        {
            /* Create render target resource */
            const CD3DX12_HEAP_PROPERTIES heapProperties{ D3D12_HEAP_TYPE_DEFAULT };
            HRESULT hr = device->CreateCommittedResource(
                &heapProperties,
                D3D12_HEAP_FLAG_NONE,
                &tex2DMSDesc,
                D3D12_RESOURCE_STATE_RENDER_TARGET,
                nullptr,
                IID_PPV_ARGS(colorBuffersMS_[i].native.ReleaseAndGetAddressOf())
            );
            DXThrowIfCreateFailed(hr, "ID3D12Resource", "for swap-chain multi-sampled color buffer");

            colorBuffersMS_[i].SetInitialState(D3D12_RESOURCE_STATE_RENDER_TARGET);
        }
    }

    /* Create render-target views */
    D3D12_CPU_DESCRIPTOR_HANDLE rtvDescHandle = rtvDescHeap_->GetCPUDescriptorHandleForHeapStart();

    for_range(i, numColorBuffers_)
    {
        D3D12Resource& renderTarget = (HasMultiSampling() ? colorBuffersMS_[i] : colorBuffers_[i]);
        device->CreateRenderTargetView(renderTarget.Get(), nullptr, rtvDescHandle);
        rtvDescHandle.ptr += rtvDescSize_;
    }
}

void D3D12SwapChain::CreateDepthStencil(ID3D12Device* device, const Extent2D& resolution)
{
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

    const CD3DX12_HEAP_PROPERTIES heapProperties{ D3D12_HEAP_TYPE_DEFAULT };
    const CD3DX12_CLEAR_VALUE clearValue{ depthStencilFormat_, 1.0f, 0 };
    HRESULT hr = device->CreateCommittedResource(
        &heapProperties,
        D3D12_HEAP_FLAG_NONE,
        &tex2DDesc,
        D3D12_RESOURCE_STATE_DEPTH_WRITE,
        &clearValue,
        IID_PPV_ARGS(depthStencil_.native.ReleaseAndGetAddressOf())
    );
    DXThrowIfCreateFailed(hr, "ID3D12Resource", "for swap-chain depth-stencil buffer");

    /* Create depth-stencil view (DSV) */
    device->CreateDepthStencilView(depthStencil_.native.Get(), nullptr, dsvDescHeap_->GetCPUDescriptorHandleForHeapStart());
}

void D3D12SwapChain::MoveToNextFrame()
{
    /* Schedule signal command into the queue */
    const UINT64 currentFenceValue = frameFenceValues_[currentColorBuffer_];
    commandQueue_->SignalFence(frameFence_.Get(), currentFenceValue);

    /* Advance frame index */
    currentColorBuffer_ = swapChainDXGI_->GetCurrentBackBufferIndex();

    /* Wait until the fence value of the next frame is signaled, so we know the next frame is ready to start */
    frameFence_.WaitForHigherSignal(frameFenceValues_[currentColorBuffer_]);
    frameFenceValues_[currentColorBuffer_] = currentFenceValue + 1;
}

void D3D12SwapChain::StoreDebugNames(std::string (&debugNames)[D3D12SwapChain::numDebugNames])
{
    for_range(i, D3D12SwapChain::maxNumColorBuffers)
    {
        debugNames[i*2    ] = D3D12GetObjectName(colorBuffers_[i].Get());
        debugNames[i*2 + 1] = D3D12GetObjectName(colorBuffersMS_[i].Get());
    }
    debugNames[D3D12SwapChain::maxNumColorBuffers*2] = D3D12GetObjectName(depthStencil_.Get());
}

void D3D12SwapChain::RestoreDebugNames(const std::string (&debugNames)[D3D12SwapChain::numDebugNames])
{
    for_range(i, D3D12SwapChain::maxNumColorBuffers)
    {
        D3D12SetObjectName(colorBuffers_[i].Get(), debugNames[i*2].c_str());
        D3D12SetObjectName(colorBuffersMS_[i].Get(), debugNames[i*2 + 1].c_str());
    }
    D3D12SetObjectName(depthStencil_.Get(), debugNames[D3D12SwapChain::maxNumColorBuffers*2].c_str());
}


} // /namespace LLGL



// ================================================================================
