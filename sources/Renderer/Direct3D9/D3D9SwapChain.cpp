/*
 * D3D9SwapChain.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D9SwapChain.h"
#include "D3D9Core.h"
#include "D3D9Types.h"
#include <LLGL/Platform/NativeHandle.h>
#include <LLGL/Utils/ForRange.h>


namespace LLGL
{


static Format ChooseColorFormat(int /*colorBits*/)
{
    return Format::RGBA8UNorm;
}

static Format ChooseDepthStencilFormat(int depthBits, int stencilBits)
{
    if (stencilBits != 0)
        return Format::D24UNormS8UInt;
    else if (depthBits == 16)
        return Format::D16UNorm;
    else
        return Format::D32Float;
}

D3D9SwapChain::D3D9SwapChain(
    IDirect3DDevice9*               device,
    const SwapChainDescriptor&      desc,
    const std::shared_ptr<Surface>& surface,
    const RendererInfo&             rendererInfo)
:
    SwapChain           { desc                            },
    numBackBuffers_     { desc.swapBuffers >= 3 ? 2u : 1u },
    samples_            { desc.samples                    }
{
    SetOrCreateSurface(surface, SwapChain::BuildDefaultSurfaceTitle(rendererInfo), desc);

    /* Get native handle */
    NativeHandle nativeHandle = {};
    GetSurface().GetNativeHandle(&nativeHandle, sizeof(nativeHandle));
    HWND deviceWindow = nativeHandle.window;

    /* Create D3D swap chain */
    D3DPRESENT_PARAMETERS presentParams = {};
    {
        presentParams.BackBufferWidth               = desc.resolution.width;
        presentParams.BackBufferHeight              = desc.resolution.height;
        presentParams.BackBufferFormat              = D3DFMT_UNKNOWN;
        presentParams.BackBufferCount               = 1 + numBackBuffers_;

        presentParams.MultiSampleType               = D3DMULTISAMPLE_NONE; //TODO
        presentParams.MultiSampleQuality            = 0;

        presentParams.SwapEffect                    = D3DSWAPEFFECT_DISCARD;
        presentParams.hDeviceWindow                 = deviceWindow;
        presentParams.Windowed                      = TRUE;
        presentParams.EnableAutoDepthStencil        = FALSE;
        presentParams.AutoDepthStencilFormat        = D3DFMT_UNKNOWN;
        presentParams.Flags                         = 0;

        presentParams.FullScreen_RefreshRateInHz    = 0; // FullScreen_RefreshRateInHz must be zero for Windowed mode
        presentParams.PresentationInterval          = D3DPRESENT_INTERVAL_DEFAULT;
    }
    HRESULT result = device->CreateAdditionalSwapChain(&presentParams, swapChain_.GetAddressOf());
    D3DThrowIfCreateFailed(result, "IDirect3DSwapChain9");

    /* Get actual presentation parameters */
    D3DPRESENT_PARAMETERS resolvedParams = {};
    if (SUCCEEDED(swapChain_->GetPresentParameters(&resolvedParams)))
        colorFormat_ = D3D9Types::ToFormat(resolvedParams.BackBufferFormat);

    /* Store references to back buffers for faster access */
    for_range(i, numBackBuffers_)
        swapChain_->GetBackBuffer(i, D3DBACKBUFFER_TYPE_MONO, backBuffers_[i].GetAddressOf());

    /* Initialize debug name */
    if (desc.debugName != nullptr)
        SetDebugName(desc.debugName);

    /* Show default surface */
    if (!surface)
        ShowSurface();
}

void D3D9SwapChain::SetDebugName(const char* name)
{
    if (name != nullptr)
        label_ = name;
    else
        label_.clear();
}

bool D3D9SwapChain::IsPresentable() const
{
    return true; // dummy
}

void D3D9SwapChain::Present()
{
    swapChain_->Present(nullptr, nullptr, nullptr, nullptr, 0);
}

std::uint32_t D3D9SwapChain::GetCurrentSwapIndex() const
{
    return 0; // dummy
}

std::uint32_t D3D9SwapChain::GetNumSwapBuffers() const
{
    return numBackBuffers_ + 1; // dummy
}

std::uint32_t D3D9SwapChain::GetSamples() const
{
    return samples_;
}

Format D3D9SwapChain::GetColorFormat() const
{
    return colorFormat_;
}

Format D3D9SwapChain::GetDepthStencilFormat() const
{
    return depthStencilFormat_;
}

bool D3D9SwapChain::SetVsyncInterval(std::uint32_t vsyncInterval)
{
    vsyncInterval_ = vsyncInterval;
    return true;
}

const RenderPass* D3D9SwapChain::GetRenderPass() const
{
    return renderPass_;
}

bool D3D9SwapChain::ResizeBuffersPrimary(const Extent2D& /*resolution*/)
{
    return true;
}


} // /namespace LLGL



// ================================================================================
