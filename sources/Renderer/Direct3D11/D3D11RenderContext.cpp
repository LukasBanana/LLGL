/*
 * D3D11RenderContext.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11RenderContext.h"
#include "D3D11RenderSystem.h"
#include <LLGL/Platform/NativeHandle.h>
#include "../../Core/Helper.h"


namespace LLGL
{


D3D11RenderContext::D3D11RenderContext(
    D3D11RenderSystem& renderSystem,
    D3D11StateManager& stateMngr,
    const ComPtr<ID3D11DeviceContext>& context,
    RenderContextDescriptor desc,
    const std::shared_ptr<Window>& window) :
        renderSystem_   ( renderSystem ),
        stateMngr_      ( stateMngr    ),
        context_        ( context      ),
        desc_           ( desc         )
{
    /* Setup window for the render context */
    SetWindow(window, desc_.videoMode, nullptr);

    /* Create D3D objects */
    CreateSwapChain();
    CreateBackBuffer(desc.videoMode.resolution.x, desc.videoMode.resolution.y);

    /* Initialize v-sync */
    SetVsync(desc_.vsync);
}

void D3D11RenderContext::Present()
{
    swapChain_->Present(swapChainInterval_, 0);
}

/* ----- Configuration ----- */

void D3D11RenderContext::SetVideoMode(const VideoModeDescriptor& videoModeDesc)
{
    if (GetVideoMode() != videoModeDesc)
    {
        auto prevVideoMode = GetVideoMode();

        /* Update window appearance and store new video mode in base function */
        RenderContext::SetVideoMode(videoModeDesc);

        /* Resize back buffer */
        if (!Gs::Equals(prevVideoMode.resolution, videoModeDesc.resolution))
        {
            auto size = videoModeDesc.resolution.Cast<UINT>();
            ResizeBackBuffer(size.x, size.y);
        }

        /* Switch fullscreen mode */
        if (prevVideoMode.fullscreen != videoModeDesc.fullscreen)
            swapChain_->SetFullscreenState(videoModeDesc.fullscreen ? TRUE : FALSE, nullptr);
    }
}

void D3D11RenderContext::SetVsync(const VsyncDescriptor& vsyncDesc)
{
    desc_.vsync = vsyncDesc;
    swapChainInterval_ = (vsyncDesc.enabled ? std::max(1u, std::min(vsyncDesc.interval, 4u)) : 0u);
}


/*
 * ======= Private: =======
 */

void D3D11RenderContext::CreateSwapChain()
{
    /* Create swap chain for window handle */
    NativeHandle wndHandle;
    GetWindow().GetNativeHandle(&wndHandle);

    DXGI_SWAP_CHAIN_DESC swapChainDesc;
    InitMemory(swapChainDesc);
    {
        swapChainDesc.BufferDesc.Width                      = desc_.videoMode.resolution.x;
        swapChainDesc.BufferDesc.Height                     = desc_.videoMode.resolution.y;
        swapChainDesc.BufferDesc.Format                     = DXGI_FORMAT_R8G8B8A8_UNORM;
        swapChainDesc.BufferDesc.RefreshRate.Numerator      = desc_.vsync.refreshRate;
        swapChainDesc.BufferDesc.RefreshRate.Denominator    = desc_.vsync.interval;
        swapChainDesc.SampleDesc.Count                      = (desc_.sampling.enabled ? std::max(1u, desc_.sampling.samples) : 1);
        swapChainDesc.SampleDesc.Quality                    = 0;
        swapChainDesc.BufferUsage                           = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        swapChainDesc.BufferCount                           = (desc_.videoMode.swapChainMode == SwapChainMode::TripleBuffering ? 2 : 1);
        swapChainDesc.OutputWindow                          = wndHandle.window;
        swapChainDesc.Windowed                              = (desc_.videoMode.fullscreen ? FALSE : TRUE);
        swapChainDesc.SwapEffect                            = DXGI_SWAP_EFFECT_DISCARD;
    }
    swapChain_ = renderSystem_.CreateDXSwapChain(swapChainDesc);
}

void D3D11RenderContext::CreateBackBuffer(UINT width, UINT height)
{
    /* Get back buffer from swap chain */
    auto hr = swapChain_->GetBuffer(0, IID_PPV_ARGS(backBuffer_.colorBuffer.ReleaseAndGetAddressOf()));
    DXThrowIfFailed(hr, "failed to get back buffer from D3D11 swap chain");

    /* Create back buffer RTV */
    hr = renderSystem_.GetDevice()->CreateRenderTargetView(backBuffer_.colorBuffer.Get(), nullptr, backBuffer_.rtv.ReleaseAndGetAddressOf());
    DXThrowIfFailed(hr, "failed to create render-target-view (RTV) for D3D11 back buffer");

    /* Create depth-stencil and DSV */
    renderSystem_.CreateDXDepthStencilAndDSV(
        width,
        height,
        (desc_.sampling.enabled ? std::max(1u, desc_.sampling.samples) : 1),
        DXGI_FORMAT_D24_UNORM_S8_UINT,
        backBuffer_.depthStencil,
        backBuffer_.dsv
    );
}

void D3D11RenderContext::ResizeBackBuffer(UINT width, UINT height)
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
    CreateBackBuffer(width, height);

    /* Reset render target (if the previous RTV and DSV was from this render context) */
    //if (rtvFromThis)
    /*{
        ID3D11RenderTargetView* rtvList[] = { backBuffer_.rtv.Get() };
        context_->OMSetRenderTargets(1, rtvList, backBuffer_.dsv.Get());
    }*/
}


} // /namespace LLGL



// ================================================================================
