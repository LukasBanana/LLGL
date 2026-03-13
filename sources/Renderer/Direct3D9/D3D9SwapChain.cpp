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
    SwapChain           { desc                                                       },
    device_             { device                                                     },
    numBackBuffers_     { (desc.swapBuffers >= 3 ? 2u : 1u)                          },
    samples_            { desc.samples                                               },
    depthStencilFormat_ { ChooseDepthStencilFormat(desc.depthBits, desc.stencilBits) }
{
    SetOrCreateSurface(surface, SwapChain::BuildDefaultSurfaceTitle(rendererInfo), desc);

    /* Initialize resolution dependent resources: D3D swap-chain with back buffers and custom depth-stencil surface */
    CreateResolutionDependentResources(desc.resolution);

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


/*
 * ======= Private: =======
 */

bool D3D9SwapChain::ResizeBuffersPrimary(const Extent2D& resolution)
{
    /* Unbind all render targets we're about to release and bind the new ones if they were active */
    constexpr int maxNumTargets = LLGL_ARRAY_LENGTH(backBuffers_);

    bool isRenderTargetBound[maxNumTargets] = {};
    for_range(i, maxNumTargets)
    {
        ComPtr<IDirect3DSurface9> renderTarget;
        device_->GetRenderTarget(i, renderTarget.GetAddressOf());
        if (renderTarget.Get() == backBuffers_[i].Get())
        {
            isRenderTargetBound[i] = true;
            device_->SetRenderTarget(i, nullptr);
        }
    }

    bool isDepthStencilBound = false;
    {
        ComPtr<IDirect3DSurface9> depthStencil;
        device_->GetDepthStencilSurface(depthStencil.GetAddressOf());
        if (depthStencil.Get() == depthStencil_.Get())
        {
            device_->SetDepthStencilSurface(false);
            isDepthStencilBound = true;
        }
    }

    /* Create new back buffers and depth-stencil surface for new resolution */
    CreateResolutionDependentResources(resolution);

    /* Bind new targets if the old ones were previously bound */
    for_range(i, maxNumTargets)
    {
        if (isRenderTargetBound[i])
            device_->SetRenderTarget(i, backBuffers_[i].Get());
    }
    if (isDepthStencilBound)
        device_->SetDepthStencilSurface(depthStencil_.Get());

    return true;
}

D3DMULTISAMPLE_TYPE D3D9SwapChain::GetMultiSampleType() const
{
    return D3DMULTISAMPLE_NONE; //TODO
}

void D3D9SwapChain::CreateResolutionDependentResources(const Extent2D& resolution)
{
    /* Get native handle */
    NativeHandle nativeHandle = {};
    GetSurface().GetNativeHandle(&nativeHandle, sizeof(nativeHandle));
    HWND deviceWindow = nativeHandle.window;

    /* Create D3D swap chain */
    D3DPRESENT_PARAMETERS presentParams = {};
    {
        presentParams.BackBufferWidth               = resolution.width;
        presentParams.BackBufferHeight              = resolution.height;
        presentParams.BackBufferFormat              = D3DFMT_UNKNOWN;
        presentParams.BackBufferCount               = 1 + numBackBuffers_;

        presentParams.MultiSampleType               = GetMultiSampleType();
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
    HRESULT result = device_->CreateAdditionalSwapChain(&presentParams, swapChain_.ReleaseAndGetAddressOf());
    D3DThrowIfCreateFailed(result, "IDirect3DSwapChain9");

    /* Get actual presentation parameters */
    D3DPRESENT_PARAMETERS resolvedParams = {};
    if (SUCCEEDED(swapChain_->GetPresentParameters(&resolvedParams)))
        colorFormat_ = D3D9Types::ToFormat(resolvedParams.BackBufferFormat);

    /* Store references to back buffers for faster access */
    for_range(i, numBackBuffers_)
        swapChain_->GetBackBuffer(i, D3DBACKBUFFER_TYPE_MONO, backBuffers_[i].ReleaseAndGetAddressOf());

    /* Create depth-stencil surface */
    if (depthStencilFormat_ != Format::Undefined)
    {
        D3DFORMAT d3dFormat = D3D9Types::ToD3DFormat(depthStencilFormat_);
        HRESULT hr = device_->CreateDepthStencilSurface(
            resolution.width,
            resolution.height,
            d3dFormat,
            GetMultiSampleType(),
            0,
            TRUE,
            depthStencil_.ReleaseAndGetAddressOf(),
            nullptr
        );
        D3DThrowIfCreateFailed(hr, "IDirect3DSurface9", "for swap-chain depth-stencil");
    }
}


} // /namespace LLGL



// ================================================================================
