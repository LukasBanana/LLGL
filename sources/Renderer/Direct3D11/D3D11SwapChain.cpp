/*
 * D3D11SwapChain.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D11SwapChain.h"
#include "D3D11RenderSystem.h"
#include "D3D11ObjectUtils.h"
#include "../DXCommon/DXTypes.h"
#include "../../Core/CoreUtils.h"
#include <LLGL/Platform/NativeHandle.h>
#include <LLGL/Log.h>


namespace LLGL
{


D3D11SwapChain::D3D11SwapChain(
    IDXGIFactory*                       factory,
    const ComPtr<ID3D11Device>&         device,
    D3D11RenderSystem&                  renderSystem,
    const SwapChainDescriptor&          desc,
    const std::shared_ptr<Surface>&     surface)
:
    SwapChain            { desc                                                       },
    device_              { device                                                     },
    renderSystem_        { renderSystem                                               },
    depthStencilFormat_  { DXPickDepthStencilFormat(desc.depthBits, desc.stencilBits) },
    renderTargetHandles_ { 1u, (depthStencilFormat_ != DXGI_FORMAT_UNKNOWN)           },
    tearingSupported_    { renderSystem.IsTearingSupported()                          },
    colorBufferLocator_  { ResourceType::Texture, BindFlags::ColorAttachment          },
    depthBufferLocator_  { ResourceType::Texture, BindFlags::DepthStencilAttachment   }
{
    /* Setup surface for the swap-chain */
    SetOrCreateSurface(surface, SwapChain::BuildDefaultSurfaceTitle(renderSystem.GetRendererInfo()), desc.resolution, desc.fullscreen);

    /* Create D3D objects */
    CreateSwapChain(factory, GetResolution(), desc.samples, desc.swapBuffers);
    CreateResolutionDependentResources();

    if (desc.debugName != nullptr)
        SetDebugName(desc.debugName);

    /* Show default surface */
    if (!surface)
        ShowSurface();
}

void D3D11SwapChain::SetDebugName(const char* name)
{
    if (name != nullptr)
    {
        /* Set label for each back-buffer object */
        D3D11SetObjectName(colorBuffer_.Get(), name);
        D3D11SetObjectNameSubscript(colorBufferMS_.Get(), name, ".MS");
        D3D11SetObjectNameSubscript(renderTargetHandles_.GetRenderTargetViews()[0], name, ".RTV");
        if (depthBuffer_)
        {
            D3D11SetObjectNameSubscript(depthBuffer_.Get(), name, ".DS");
            D3D11SetObjectNameSubscript(renderTargetHandles_.GetDepthStencilView(), name, ".DSV");
        }
        hasDebugName_ = true;
    }
    else
    {
        /* Reset all back-buffer labels */
        D3D11SetObjectName(colorBuffer_.Get(), nullptr);
        D3D11SetObjectName(colorBufferMS_.Get(), nullptr);
        D3D11SetObjectName(renderTargetHandles_.GetRenderTargetViews()[0], nullptr);
        if (depthBuffer_)
        {
            D3D11SetObjectName(depthBuffer_.Get(), nullptr);
            D3D11SetObjectName(renderTargetHandles_.GetDepthStencilView(), nullptr);
        }
        hasDebugName_ = false;
    }
}

bool D3D11SwapChain::IsPresentable() const
{
    return true; // dummy
}

void D3D11SwapChain::Present()
{
    const bool tearingEnabled   = (tearingSupported_ && windowedMode_ && swapChainInterval_ == 0);
    const UINT presentFlags     = (tearingEnabled ? DXGI_PRESENT_ALLOW_TEARING : 0u);
    swapChain_->Present(swapChainInterval_, presentFlags);
}

std::uint32_t D3D11SwapChain::GetCurrentSwapIndex() const
{
    return 0; // dummy
}

std::uint32_t D3D11SwapChain::GetNumSwapBuffers() const
{
    return 1; // dummy
}

std::uint32_t D3D11SwapChain::GetSamples() const
{
    return swapChainSampleDesc_.Count;
}

Format D3D11SwapChain::GetColorFormat() const
{
    return DXTypes::Unmap(colorFormat_);
}

Format D3D11SwapChain::GetDepthStencilFormat() const
{
    return DXTypes::Unmap(depthStencilFormat_);
}

const RenderPass* D3D11SwapChain::GetRenderPass() const
{
    return nullptr; // dummy
}

bool D3D11SwapChain::SetVsyncInterval(std::uint32_t vsyncInterval)
{
    return SetPresentSyncInterval(vsyncInterval);
}

static bool IsD3D11BoxCoveringWholeResource(UINT width, UINT height, const D3D11_BOX& box)
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

static void D3D11GetResourceTypeAndSampleCount(ID3D11Resource* resource, D3D11_RESOURCE_DIMENSION& outDimension, UINT& outSampleCount)
{
    resource->GetType(&outDimension);
    if (outDimension == D3D11_RESOURCE_DIMENSION_TEXTURE2D)
    {
        D3D11_TEXTURE2D_DESC texDesc;
        static_cast<ID3D11Texture2D*>(resource)->GetDesc(&texDesc);
        outSampleCount = texDesc.SampleDesc.Count;
    }
    else
        outSampleCount = 1;
}

static void D3D11CopyFramebufferSubresourceRegion(
    ID3D11DeviceContext*    context,
    ID3D11Resource*         dstResource,
    UINT                    dstSubresource,
    UINT                    dstX,
    UINT                    dstY,
    UINT                    dstZ,
    ID3D11Texture2D*        srcResource,
    const D3D11_BOX&        srcBox)
{
    /* Check if whole resource must be copied with an intermediate texture */
    D3D11_TEXTURE2D_DESC srcResourceDesc = {};
    srcResource->GetDesc(&srcResourceDesc);

    D3D11_RESOURCE_DIMENSION dstResourceType = D3D11_RESOURCE_DIMENSION_UNKNOWN;
    UINT dstResourceSampleCount = 0;
    D3D11GetResourceTypeAndSampleCount(dstResource, dstResourceType, dstResourceSampleCount);

    /* Multisampled or depth-stencil resources must be copied as a whole */
    const bool isSrcMultisampled = (srcResourceDesc.SampleDesc.Count > 1);
    const bool isDepthStencil = ((srcResourceDesc.BindFlags & D3D11_BIND_DEPTH_STENCIL) != 0);

    if (isDepthStencil || isSrcMultisampled)
    {
        const bool isDstOffsetZero = (dstX == 0 && dstY == 0 && dstZ == 0);
        const bool isWholeResource = (isDstOffsetZero && IsD3D11BoxCoveringWholeResource(srcResourceDesc.Width, srcResourceDesc.Height, srcBox));
        if (isWholeResource && dstResourceSampleCount == srcResourceDesc.SampleDesc.Count)
        {
            /* Copy whole subresource */
            context->CopySubresourceRegion(dstResource, dstSubresource, 0, 0, 0, srcResource, 0, nullptr);
        }
        else
        {
            /* Create intermediate texture */
            ComPtr<ID3D11Device> device;
            srcResource->GetDevice(device.GetAddressOf());
            LLGL_ASSERT_PTR(device.Get());

            ComPtr<ID3D11Texture2D> intermediateTex;
            {
                srcResourceDesc.BindFlags   = D3D11_BIND_RENDER_TARGET;
                srcResourceDesc.MipLevels   = 1;
                srcResourceDesc.SampleDesc  = { 1, 0 };
            }
            HRESULT hr = device->CreateTexture2D(&srcResourceDesc, nullptr, intermediateTex.GetAddressOf());
            DXThrowIfCreateFailed(hr, "ID3D11Texture2D", "for intermediate framebuffer");

            if (isSrcMultisampled)
                context->ResolveSubresource(intermediateTex.Get(), 0, srcResource, 0, srcResourceDesc.Format);
            else
                context->CopySubresourceRegion(intermediateTex.Get(), 0, 0, 0, 0, srcResource, 0, nullptr);

            /* Copy intermediate texture into destination resource */
            context->CopySubresourceRegion(dstResource, dstSubresource, dstX, dstY, dstZ, intermediateTex.Get(), 0, &srcBox);
        }
    }
    else
    {
        /* Copy subresource region directly */
        context->CopySubresourceRegion(dstResource, dstSubresource, dstX, dstY, dstZ, srcResource, 0, &srcBox);
    }
}

HRESULT D3D11SwapChain::CopySubresourceRegion(
    ID3D11DeviceContext*    context,
    ID3D11Resource*         dstResource,
    UINT                    dstSubresource,
    UINT                    dstX,
    UINT                    dstY,
    UINT                    dstZ,
    const D3D11_BOX&        srcBox,
    DXGI_FORMAT             format)
{
    const bool isDepthStencil = DXTypes::IsDepthStencilDXGIFormat(DXTypes::ToDXGIFormatDSV(format));
    if (isDepthStencil)
    {
        if (!depthBuffer_)
            return E_FAIL;
        if (DXTypes::ToDXGIFormatTypeless(depthStencilFormat_) != DXTypes::ToDXGIFormatTypeless(format))
            return E_INVALIDARG;
        D3D11CopyFramebufferSubresourceRegion(context, dstResource, dstSubresource, dstX, dstY, dstZ, depthBuffer_.Get(), srcBox);
    }
    else
    {
        if (!colorBuffer_)
            return E_FAIL;
        if (DXTypes::ToDXGIFormatTypeless(colorFormat_) != DXTypes::ToDXGIFormatTypeless(format))
            return E_INVALIDARG;
        D3D11CopyFramebufferSubresourceRegion(context, dstResource, dstSubresource, dstX, dstY, dstZ, colorBuffer_.Get(), srcBox);
    }
    return S_OK;
}

void D3D11SwapChain::ResolveSubresources(ID3D11DeviceContext* context)
{
    if (colorBufferMS_.Get() != nullptr)
        context->ResolveSubresource(colorBuffer_.Get(), 0, colorBufferMS_.Get(), 0, colorFormat_);
}


/*
 * ======= Private: =======
 */

bool D3D11SwapChain::ResizeBuffersPrimary(const Extent2D& resolution)
{
    /* Store current debug names */
    std::string debugNames[5];
    if (hasDebugName_)
        StoreDebugNames(debugNames);

    /* Unset render targets for last used command buffer context */
    renderSystem_.ClearStateForAllContexts();

    /* Release buffers */
    colorBuffer_.Reset();
    colorBufferMS_.Reset();
    depthBuffer_.Reset();
    renderTargetHandles_.Reset();

    /* Resize swap-chain buffers, let DXGI find out the client area, and preserve buffer count and format */
    DXGI_SWAP_CHAIN_DESC desc;
    swapChain_->GetDesc(&desc);

    HRESULT hr = swapChain_->ResizeBuffers(0, resolution.width, resolution.height, DXGI_FORMAT_UNKNOWN, desc.Flags);
    DXThrowIfFailed(hr, "failed to resize DXGI swap-chain buffers");

    /* Update windowed mode */
    windowedMode_ = !DXGetFullscreenState(swapChain_.Get());

    /* Recreate back buffer and reset default render target */
    CreateResolutionDependentResources();

    /* Restore debug names with new swap-chain buffers */
    if (hasDebugName_)
        RestoreDebugNames(debugNames);

    return true;
}

bool D3D11SwapChain::SetPresentSyncInterval(UINT syncInterval)
{
    /* IDXGISwapChain::Present expects a sync interval in the range [0, 4] */
    if (syncInterval <= 4)
    {
        swapChainInterval_ = syncInterval;
        return true;
    }
    return false;
}

static UINT GetPrimaryDisplayRefreshRate()
{
    if (auto display = Display::GetPrimary())
        return display->GetDisplayMode().refreshRate;
    else
        return 60; // Assume most common refresh rate
}

void D3D11SwapChain::CreateSwapChain(IDXGIFactory* factory, const Extent2D& resolution, std::uint32_t samples, std::uint32_t swapBuffers)
{
    HRESULT hr = S_OK;

    /* Pick and store color format */
    colorFormat_ = DXGI_FORMAT_R8G8B8A8_UNORM;//DXGI_FORMAT_B8G8R8A8_UNORM

    /* Create swap chain for window handle */
    NativeHandle wndHandle = {};
    GetSurface().GetNativeHandle(&wndHandle, sizeof(wndHandle));

    /* Find suitable multi-samples for color format */
    swapChainSampleDesc_ = D3D11RenderSystem::FindSuitableSampleDesc(device_.Get(), colorFormat_, samples);

    #ifdef LLGL_OS_UWP
    /* Create IDXGISwapChain1 with IDXGIFactory2 for CoreWindow */
    ComPtr<IDXGIFactory2> factory2;
    hr = factory->QueryInterface(IID_PPV_ARGS(&factory2));
    DXThrowIfFailed(hr, "failed to query IDXGIFactory2");
    CreateDXGISwapChain1(factory2.Get(), wndHandle, resolution, swapBuffers);
    #else // LLGL_OS_UWP
    #if LLGL_D3D11_ENABLE_FEATURELEVEL >= 3
    ComPtr<IDXGIFactory2> factory2;
    hr = factory->QueryInterface(IID_PPV_ARGS(&factory2));

    if (SUCCEEDED(hr))
    {
        /* Create IDXGISwapChain1 with IDXGIFactory2 to support tearing */
        CreateDXGISwapChain1(factory2.Get(), wndHandle, resolution, swapBuffers);
    }
    else
    #endif
    {
        /* Create IDXGISwapChain with IDXGIFactory */
        CreateDXGISwapChain(factory, wndHandle, resolution, swapBuffers, samples);
    }
    #endif // /LLGL_OS_UWP

    /* Cache windoed mode for tearing support */
    windowedMode_ = !DXGetFullscreenState(swapChain_.Get());
}

#ifdef LLGL_OS_WIN32

void D3D11SwapChain::CreateDXGISwapChain(IDXGIFactory* factory, const NativeHandle& wndHandle, const Extent2D& resolution, std::uint32_t swapBuffers, std::uint32_t samples)
{
    /* Clamp buffer count between 1 and max buffers */
    swapBuffers = std::max(1u, std::min<std::uint32_t>(swapBuffers, DXGI_MAX_SWAP_CHAIN_BUFFERS));

    const DXGI_RATIONAL refreshRate{ GetPrimaryDisplayRefreshRate(), 1 };

    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    {
        swapChainDesc.BufferDesc.Width          = resolution.width;
        swapChainDesc.BufferDesc.Height         = resolution.height;
        swapChainDesc.BufferDesc.Format         = colorFormat_;
        swapChainDesc.BufferDesc.RefreshRate    = refreshRate;
        swapChainDesc.SampleDesc                = swapChainSampleDesc_;
        swapChainDesc.BufferUsage               = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount               = (swapBuffers >= 3 ? 2 : 1);
        swapChainDesc.OutputWindow              = wndHandle.window;
        swapChainDesc.Windowed                  = TRUE;//(fullscreen ? FALSE : TRUE);
        swapChainDesc.SwapEffect                = DXGI_SWAP_EFFECT_DISCARD;
    }

    HRESULT hr = factory->CreateSwapChain(device_.Get(), &swapChainDesc, swapChain_.ReleaseAndGetAddressOf());
    DXThrowIfFailed(hr, "failed to create DXGI swap chain");

    swapEffectFlip_ = false;
}

#endif // /LLGL_OS_WIN32

#if LLGL_D3D11_ENABLE_FEATURELEVEL >= 3 || defined LLGL_OS_UWP

void D3D11SwapChain::CreateDXGISwapChain1(IDXGIFactory2* factory2, const NativeHandle& wndHandle, const Extent2D& resolution, std::uint32_t swapBuffers)
{
    /* Clamp buffer count between 2 and max buffers */
    swapBuffers = std::max(2u, std::min<std::uint32_t>(swapBuffers, DXGI_MAX_SWAP_CHAIN_BUFFERS));

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    {
        swapChainDesc.Width                 = resolution.width;
        swapChainDesc.Height                = resolution.height;
        swapChainDesc.Format                = colorFormat_;
        swapChainDesc.SampleDesc.Count      = 1;
        swapChainDesc.SampleDesc.Quality    = 0;
        swapChainDesc.BufferUsage           = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount           = swapBuffers;
        swapChainDesc.SwapEffect            = DXGI_SWAP_EFFECT_FLIP_DISCARD; // FLIP effect requires BufferCount >= 2 && SampleDesc.Count == 1
        swapChainDesc.Flags                 = (tearingSupported_ ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0u);
    }

    ComPtr<IDXGISwapChain1> swapChain;
    #ifdef LLGL_OS_UWP
    HRESULT hr = factory2->CreateSwapChainForCoreWindow(device_.Get(), wndHandle.window, &swapChainDesc, nullptr, &swapChain);
    #else
    HRESULT hr = factory2->CreateSwapChainForHwnd(device_.Get(), wndHandle.window, &swapChainDesc, nullptr, nullptr, &swapChain);
    #endif
    DXThrowIfFailed(hr, "failed to create DXGI swap chain");
    DXThrowIfFailed(swapChain.As(&swapChain_), "failed to downcast swap chain");

    swapEffectFlip_ = true;
}

#endif

void D3D11SwapChain::CreateResolutionDependentResources()
{
    HRESULT hr = 0;

    /* Get back buffer from swap chain (must always be zero for DXGI_SWAP_EFFECT_DISCARD) */
    hr = swapChain_->GetBuffer(0, IID_PPV_ARGS(colorBuffer_.ReleaseAndGetAddressOf()));
    DXThrowIfFailed(hr, "failed to get D3D11 back buffer from swap chain");

    /* Retrieve back-buffer dimension */
    D3D11_TEXTURE2D_DESC colorBufferDesc = {};
    colorBuffer_->GetDesc(&colorBufferDesc);

    /* If swap-effect is FLIP and multi-sampling is enabled, we have to create our own multi-sampled back-buffer */
    ComPtr<ID3D11RenderTargetView> rtv;
    if (swapEffectFlip_ && swapChainSampleDesc_.Count > 1)
    {
        /* Create multi-sampled texture */
        D3D11_TEXTURE2D_DESC texDesc;
        {
            texDesc.Width           = colorBufferDesc.Width;
            texDesc.Height          = colorBufferDesc.Height;
            texDesc.MipLevels       = 1;
            texDesc.ArraySize       = 1;
            texDesc.Format          = colorFormat_;
            texDesc.SampleDesc      = swapChainSampleDesc_;
            texDesc.Usage           = D3D11_USAGE_DEFAULT;
            texDesc.BindFlags       = D3D11_BIND_RENDER_TARGET;
            texDesc.CPUAccessFlags  = 0;
            texDesc.MiscFlags       = 0;
        }
        hr = device_->CreateTexture2D(&texDesc, nullptr, colorBufferMS_.ReleaseAndGetAddressOf());
        DXThrowIfFailed(hr, "failed to create D3D11 multi-sampled back-buffer for swap-chain");

        /* Create back buffer RTV */
        hr = device_->CreateRenderTargetView(colorBufferMS_.Get(), nullptr, rtv.GetAddressOf());
        DXThrowIfFailed(hr, "failed to create D3D11 render-target-view (RTV) for multi-sampled back buffer");
    }
    else
    {
        /* Create back buffer RTV */
        hr = device_->CreateRenderTargetView(colorBuffer_.Get(), nullptr, rtv.GetAddressOf());
        DXThrowIfFailed(hr, "failed to create D3D11 render-target-view (RTV) for back buffer");
    }
    renderTargetHandles_.SetRenderTargetView(0, rtv.Get(), &colorBufferLocator_);

    if (depthStencilFormat_ != DXGI_FORMAT_UNKNOWN)
    {
        /* Create depth stencil texture */
        D3D11_TEXTURE2D_DESC texDesc;
        {
            texDesc.Width           = colorBufferDesc.Width;
            texDesc.Height          = colorBufferDesc.Height;
            texDesc.MipLevels       = 1;
            texDesc.ArraySize       = 1;
            texDesc.Format          = depthStencilFormat_;
            texDesc.SampleDesc      = swapChainSampleDesc_;
            texDesc.Usage           = D3D11_USAGE_DEFAULT;
            texDesc.BindFlags       = D3D11_BIND_DEPTH_STENCIL;
            texDesc.CPUAccessFlags  = 0;
            texDesc.MiscFlags       = 0;
        }
        hr = device_->CreateTexture2D(&texDesc, nullptr, depthBuffer_.ReleaseAndGetAddressOf());
        DXThrowIfFailed(hr, "failed to create D3D11 depth-texture for swap-chain");

        /* Create DSV */
        ComPtr<ID3D11DepthStencilView> dsv;
        hr = device_->CreateDepthStencilView(depthBuffer_.Get(), nullptr, dsv.GetAddressOf());
        DXThrowIfFailed(hr, "failed to create D3D11 depth-stencil-view (DSV) for swap-chain");
        renderTargetHandles_.SetDepthStencilView(dsv.Get(), &depthBufferLocator_);
    }
}

void D3D11SwapChain::StoreDebugNames(std::string (&debugNames)[5])
{
    debugNames[0] = D3D11GetObjectName(colorBuffer_.Get());
    debugNames[1] = D3D11GetObjectName(colorBufferMS_.Get());
    debugNames[2] = D3D11GetObjectName(renderTargetHandles_.GetRenderTargetViews()[0]);
    if (depthBuffer_)
    {
        debugNames[3] = D3D11GetObjectName(depthBuffer_.Get());
        debugNames[4] = D3D11GetObjectName(renderTargetHandles_.GetDepthStencilView());
    }
}

void D3D11SwapChain::RestoreDebugNames(const std::string (&debugNames)[5])
{
    D3D11SetObjectName(colorBuffer_.Get(), debugNames[0].c_str());
    D3D11SetObjectName(colorBufferMS_.Get(), debugNames[1].c_str());
    D3D11SetObjectName(renderTargetHandles_.GetRenderTargetViews()[0], debugNames[2].c_str());
    if (depthBuffer_)
    {
        D3D11SetObjectName(depthBuffer_.Get(), debugNames[3].c_str());
        D3D11SetObjectName(renderTargetHandles_.GetDepthStencilView(), debugNames[4].c_str());
    }
}


} // /namespace LLGL



// ================================================================================
