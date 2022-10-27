/*
 * D3D11SwapChain.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11SwapChain.h"
#include "D3D11RenderSystem.h"
#include "D3D11ObjectUtils.h"
#include "../DXCommon/DXTypes.h"
#include "../../Core/Helper.h"
#include <LLGL/Platform/NativeHandle.h>
#include <LLGL/Log.h>


namespace LLGL
{


D3D11RenderContext::D3D11RenderContext(
    IDXGIFactory*                   factory,
    const ComPtr<ID3D11Device>&     device,
    const SwapChainDescriptor&      desc,
    const std::shared_ptr<Surface>& surface)
:
    RenderContext       { desc                                                       },
    device_             { device                                                     },
    depthStencilFormat_ { DXPickDepthStencilFormat(desc.depthBits, desc.stencilBits) }
{
    /* Setup surface for the render context */
    SetOrCreateSurface(surface, desc.resolution, desc.fullscreen, nullptr);

    /* Create D3D objects */
    CreateSwapChain(factory, desc.resolution, desc.samples, desc.swapBuffers);
    CreateBackBuffer();
}

void D3D11RenderContext::SetName(const char* name)
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

void D3D11RenderContext::Present()
{
    swapChain_->Present(swapChainInterval_, 0);
}

std::uint32_t D3D11RenderContext::GetSamples() const
{
    return swapChainSampleDesc_.Count;
}

Format D3D11RenderContext::GetColorFormat() const
{
    return DXTypes::Unmap(colorFormat_);
}

Format D3D11RenderContext::GetDepthStencilFormat() const
{
    return DXTypes::Unmap(depthStencilFormat_);
}

const RenderPass* D3D11RenderContext::GetRenderPass() const
{
    return nullptr; // dummy
}

bool D3D11RenderContext::SetVsyncInterval(std::uint32_t vsyncInterval)
{
    return SetPresentSyncInterval(vsyncInterval);
}

void D3D11RenderContext::BindFramebufferView(D3D11CommandBuffer* commandBuffer)
{
    /* Bind framebuffer of this swap-chain in command buffer */
    if (commandBuffer != nullptr)
        commandBuffer->BindFramebufferView(1, renderTargetView_.GetAddressOf(), depthStencilView_.Get());

    /* Store reference to last used command buffer */
    bindingCommandBuffer_ = commandBuffer;
}


/*
 * ======= Private: =======
 */

bool D3D11RenderContext::ResizeBuffersPrimary(const Extent2D& resolution)
{
    ResizeBackBuffer(resolution);
    return true;
}

bool D3D11RenderContext::SetPresentSyncInterval(UINT syncInterval)
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

void D3D11RenderContext::CreateSwapChain(IDXGIFactory* factory, const Extent2D& resolution, std::uint32_t samples, std::uint32_t swapBuffers)
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

    DXGI_SWAP_CHAIN_DESC swapChainDesc;
    InitMemory(swapChainDesc);
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
    auto hr = factory->CreateSwapChain(device_.Get(), &swapChainDesc, swapChain_.ReleaseAndGetAddressOf());
    DXThrowIfFailed(hr, "failed to create DXGI swap chain");
}

void D3D11RenderContext::CreateBackBuffer()
{
    HRESULT hr = 0;

    /* Get back buffer from swap chain */
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

void D3D11RenderContext::ResizeBackBuffer(const Extent2D& resolution)
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
    auto hr = swapChain_->ResizeBuffers(0, resolution.width, resolution.height, DXGI_FORMAT_UNKNOWN, 0);
    DXThrowIfFailed(hr, "failed to resize DXGI swap-chain buffers");

    /* Recreate back buffer and reset default render target */
    CreateBackBuffer();
}


} // /namespace LLGL



// ================================================================================
