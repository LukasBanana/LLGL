/*
 * D3D11RenderContext.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11RenderContext.h"
#include "D3D11RenderSystem.h"
#include "D3D11ObjectUtils.h"
#include "../DXCommon/DXTypes.h"
#include "../../Core/Helper.h"
#include <LLGL/Platform/NativeHandle.h>
#include <LLGL/Log.h>


namespace LLGL
{


D3D11RenderContext::D3D11RenderContext(
    IDXGIFactory* factory,
    const ComPtr<ID3D11Device>& device,
    const ComPtr<ID3D11DeviceContext>& context,
    const RenderContextDescriptor& desc,
    const std::shared_ptr<Surface>& surface)
:
    RenderContext { desc.videoMode, desc.vsync },
    device_       { device                     },
    context_      { context                    }
{
    /* Setup surface for the render context */
    SetOrCreateSurface(surface, desc.videoMode, nullptr);

    /* Create D3D objects */
    CreateSwapChain(factory, desc.samples);
    CreateBackBuffer(GetVideoMode());

    /* Initialize v-sync */
    OnSetVsync(desc.vsync);
}

void D3D11RenderContext::SetName(const char* name)
{
    if (name != nullptr)
    {
        /* Set label for each back-buffer object */
        D3D11SetObjectName(backBuffer_.colorBuffer.Get(), name);
        D3D11SetObjectNameSubscript(backBuffer_.rtv.Get(), name, ".RTV");
        D3D11SetObjectNameSubscript(backBuffer_.depthStencil.Get(), name, ".DS");
        D3D11SetObjectNameSubscript(backBuffer_.dsv.Get(), name, ".DSV");
    }
    else
    {
        /* Reset all back-buffer labels */
        D3D11SetObjectName(backBuffer_.colorBuffer.Get(), nullptr);
        D3D11SetObjectName(backBuffer_.rtv.Get(), nullptr);
        D3D11SetObjectName(backBuffer_.depthStencil.Get(), nullptr);
        D3D11SetObjectName(backBuffer_.dsv.Get(), nullptr);
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

//TODO: depth-stencil and color format does not change, only resizing is considered!
bool D3D11RenderContext::OnSetVideoMode(const VideoModeDescriptor& videoModeDesc)
{
    const auto& prevVideoMode = GetVideoMode();

    /* Resize back buffer */
    if (prevVideoMode.resolution != videoModeDesc.resolution)
        ResizeBackBuffer(videoModeDesc);

    /* Switch fullscreen mode */
    #if 0
    if (prevVideoMode.fullscreen != videoModeDesc.fullscreen)
    {
        auto hr = swapChain_->SetFullscreenState(videoModeDesc.fullscreen ? TRUE : FALSE, nullptr);
        return SUCCEEDED(hr);
    }
    #else
    /* Switch fullscreen mode */
    if (!SetDisplayFullscreenMode(videoModeDesc))
        return false;
    #endif

    return true;
}

bool D3D11RenderContext::OnSetVsync(const VsyncDescriptor& vsyncDesc)
{
    swapChainInterval_ = (vsyncDesc.enabled ? std::max(1u, std::min(vsyncDesc.interval, 4u)) : 0u);
    return true;
}


/*
 * ======= Private: =======
 */

void D3D11RenderContext::CreateSwapChain(IDXGIFactory* factory, UINT samples)
{
    /* Get current settings */
    const auto& videoMode = GetVideoMode();
    const auto& vsync = GetVsync();

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
        swapChainDesc.BufferDesc.Width                      = videoMode.resolution.width;
        swapChainDesc.BufferDesc.Height                     = videoMode.resolution.height;
        swapChainDesc.BufferDesc.Format                     = colorFormat_;
        swapChainDesc.BufferDesc.RefreshRate.Numerator      = vsync.refreshRate;
        swapChainDesc.BufferDesc.RefreshRate.Denominator    = vsync.interval;
        swapChainDesc.SampleDesc                            = swapChainSampleDesc_;
        swapChainDesc.BufferUsage                           = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount                           = (videoMode.swapChainSize == 3 ? 2 : 1);
        swapChainDesc.OutputWindow                          = wndHandle.window;
        swapChainDesc.Windowed                              = TRUE;//(videoMode.fullscreen ? FALSE : TRUE);
        swapChainDesc.SwapEffect                            = DXGI_SWAP_EFFECT_DISCARD;
    }
    auto hr = factory->CreateSwapChain(device_.Get(), &swapChainDesc, swapChain_.ReleaseAndGetAddressOf());
    DXThrowIfFailed(hr, "failed to create DXGI swap chain");
}

void D3D11RenderContext::CreateBackBuffer(const VideoModeDescriptor& videoModeDesc)
{
    HRESULT hr = 0;

    /* Pick and store depth-stencil format */
    depthStencilFormat_ = DXPickDepthStencilFormat(videoModeDesc.depthBits, videoModeDesc.stencilBits);

    /* Get back buffer from swap chain */
    hr = swapChain_->GetBuffer(0, IID_PPV_ARGS(backBuffer_.colorBuffer.ReleaseAndGetAddressOf()));
    DXThrowIfFailed(hr, "failed to get D3D11 back buffer from swap chain");

    /* Create back buffer RTV */
    hr = device_->CreateRenderTargetView(backBuffer_.colorBuffer.Get(), nullptr, backBuffer_.rtv.ReleaseAndGetAddressOf());
    DXThrowIfFailed(hr, "failed to create D3D11 render-target-view (RTV) for back buffer");

    #if 0//TODO: allow to avoid a depth-buffer
    if (videoModeDesc.depthBits > 0 || videoModeDesc.stencilBits > 0)
    #endif
    {
        /* Create depth stencil texture */
        D3D11_TEXTURE2D_DESC texDesc;
        {
            texDesc.Width           = videoModeDesc.resolution.width;
            texDesc.Height          = videoModeDesc.resolution.height;
            texDesc.MipLevels       = 1;
            texDesc.ArraySize       = 1;
            texDesc.Format          = depthStencilFormat_;
            texDesc.SampleDesc      = swapChainSampleDesc_;
            texDesc.Usage           = D3D11_USAGE_DEFAULT;
            texDesc.BindFlags       = D3D11_BIND_DEPTH_STENCIL;
            texDesc.CPUAccessFlags  = 0;
            texDesc.MiscFlags       = 0;
        }
        hr = device_->CreateTexture2D(&texDesc, nullptr, backBuffer_.depthStencil.ReleaseAndGetAddressOf());
        DXThrowIfFailed(hr, "failed to create D3D11 depth-texture for swap-chain");

        /* Create DSV */
        hr = device_->CreateDepthStencilView(backBuffer_.depthStencil.Get(), nullptr, backBuffer_.dsv.ReleaseAndGetAddressOf());
        DXThrowIfFailed(hr, "failed to create D3D11 depth-stencil-view (DSV) for swap-chain");
    }
}

void D3D11RenderContext::ResizeBackBuffer(const VideoModeDescriptor& videoModeDesc)
{
    /* Check if the current RTV and DSV is from this render context */
    /*ID3D11RenderTargetView* rtv = nullptr;
    ID3D11DepthStencilView* dsv = nullptr;

    context_->OMGetRenderTargets(1, &rtv, &dsv);

    bool rtvFromThis = (rtv == backBuffer_.rtv.Get() && dsv == backBuffer_.dsv.Get());

    auto r1 = rtv->Release();
    auto r2 = dsv->Release();*/

    /* Unset render targets (if the current RTV and DSV was from this render context) */
    //if (rtvFromThis)
        context_->OMSetRenderTargets(0, nullptr, nullptr);

    /* Release buffers */
    backBuffer_.colorBuffer.Reset();
    backBuffer_.rtv.Reset();
    backBuffer_.depthStencil.Reset();
    backBuffer_.dsv.Reset();

    /* Resize swap-chain buffers, let DXGI find out the client area, and preserve buffer count and format */
    auto hr = swapChain_->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
    DXThrowIfFailed(hr, "failed to resize DXGI swap-chain buffers");

    /* Recreate back buffer and reset default render target */
    CreateBackBuffer(videoModeDesc);

    /* Reset render target (if the previous RTV and DSV was from this render context) */
    //if (rtvFromThis)
    /*{
        ID3D11RenderTargetView* rtvList[] = { backBuffer_.rtv.Get() };
        context_->OMSetRenderTargets(1, rtvList, backBuffer_.dsv.Get());
    }*/
}


} // /namespace LLGL



// ================================================================================
