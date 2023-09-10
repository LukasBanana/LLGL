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
    IDXGIFactory*                   factory,
    const ComPtr<ID3D11Device>&     device,
    const SwapChainDescriptor&      desc,
    const std::shared_ptr<Surface>& surface)
:
    SwapChain           { desc                                                       },
    device_             { device                                                     },
    depthStencilFormat_ { DXPickDepthStencilFormat(desc.depthBits, desc.stencilBits) }
{
    /* Setup surface for the swap-chain */
    SetOrCreateSurface(surface, desc.resolution, desc.fullscreen, nullptr);

    /* Create D3D objects */
    CreateSwapChain(factory, desc.resolution, desc.samples, desc.swapBuffers);
    CreateBackBuffer();
}

void D3D11SwapChain::SetName(const char* name)
{
    if (name != nullptr)
    {
        /* Set label for each back-buffer object */
        D3D11SetObjectName(colorBuffer_.Get(), name);
        D3D11SetObjectNameSubscript(renderTargetView_.Get(), name, ".RTV");
        if (depthBuffer_)
        {
            D3D11SetObjectNameSubscript(depthBuffer_.Get(), name, ".DS");
            D3D11SetObjectNameSubscript(depthStencilView_.Get(), name, ".DSV");
        }
    }
    else
    {
        /* Reset all back-buffer labels */
        D3D11SetObjectName(colorBuffer_.Get(), nullptr);
        D3D11SetObjectName(renderTargetView_.Get(), nullptr);
        if (depthBuffer_)
        {
            D3D11SetObjectName(depthBuffer_.Get(), nullptr);
            D3D11SetObjectName(depthStencilView_.Get(), nullptr);
        }
    }
}

void D3D11SwapChain::Present()
{
    swapChain_->Present(swapChainInterval_, 0);
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

void D3D11SwapChain::BindFramebufferView(D3D11CommandBuffer* commandBuffer)
{
    /* Bind framebuffer of this swap-chain in command buffer */
    if (commandBuffer != nullptr)
        commandBuffer->BindFramebufferView(1, renderTargetView_.GetAddressOf(), depthStencilView_.Get());

    /* Store reference to last used command buffer */
    bindingCommandBuffer_ = commandBuffer;
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


/*
 * ======= Private: =======
 */

bool D3D11SwapChain::ResizeBuffersPrimary(const Extent2D& resolution)
{
    ResizeBackBuffer(resolution);
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
    /* Get current settings */
    const DXGI_RATIONAL refreshRate{ GetPrimaryDisplayRefreshRate(), 1 };

    /* Pick and store color format */
    colorFormat_ = DXGI_FORMAT_R8G8B8A8_UNORM;//DXGI_FORMAT_B8G8R8A8_UNORM

    /* Find suitable multi-samples for color format */
    swapChainSampleDesc_ = D3D11RenderSystem::FindSuitableSampleDesc(device_.Get(), colorFormat_, samples);

    /* Create swap chain for window handle */
    NativeHandle wndHandle = {};
    GetSurface().GetNativeHandle(&wndHandle, sizeof(wndHandle));

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
}

void D3D11SwapChain::CreateBackBuffer()
{
    HRESULT hr = 0;

    /* Get back buffer from swap chain (must always be zero for DXGI_SWAP_EFFECT_DISCARD) */
    hr = swapChain_->GetBuffer(0, IID_PPV_ARGS(colorBuffer_.ReleaseAndGetAddressOf()));
    DXThrowIfFailed(hr, "failed to get D3D11 back buffer from swap chain");

    /* Create back buffer RTV */
    hr = device_->CreateRenderTargetView(colorBuffer_.Get(), nullptr, renderTargetView_.ReleaseAndGetAddressOf());
    DXThrowIfFailed(hr, "failed to create D3D11 render-target-view (RTV) for back buffer");

    /* Retrieve back-buffer dimension */
    D3D11_TEXTURE2D_DESC colorBufferDesc;
    colorBuffer_->GetDesc(&colorBufferDesc);

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
        hr = device_->CreateDepthStencilView(depthBuffer_.Get(), nullptr, depthStencilView_.ReleaseAndGetAddressOf());
        DXThrowIfFailed(hr, "failed to create D3D11 depth-stencil-view (DSV) for swap-chain");
    }
}

void D3D11SwapChain::ResizeBackBuffer(const Extent2D& resolution)
{
    /* Unset render targets for last used command buffer context */
    if (bindingCommandBuffer_ != nullptr)
        bindingCommandBuffer_->BindFramebufferView(0, nullptr, nullptr);

    /* Release buffers */
    colorBuffer_.Reset();
    renderTargetView_.Reset();
    depthBuffer_.Reset();
    depthStencilView_.Reset();

    /* Reset command list for deferred device contexts to ensure all outstanding references to the backbuffer are cleared */
    if (bindingCommandBuffer_ != nullptr)
        bindingCommandBuffer_->ResetDeferredCommandList();

    /* Resize swap-chain buffers, let DXGI find out the client area, and preserve buffer count and format */
    HRESULT hr = swapChain_->ResizeBuffers(0, resolution.width, resolution.height, DXGI_FORMAT_UNKNOWN, 0);
    DXThrowIfFailed(hr, "failed to resize DXGI swap-chain buffers");

    /* Recreate back buffer and reset default render target */
    CreateBackBuffer();
}


} // /namespace LLGL



// ================================================================================
