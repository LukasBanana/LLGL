/*
 * WGSwapChain.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "WGSwapChain.h"
#include "WGRenderSystem.h"
#include "WGCore.h"
#include "../../Core/Assertion.h"
#include <LLGL/Platform/NativeHandle.h>


namespace LLGL
{


static WGPUTextureFormat PickDepthStencilFormat(int depthBits, int stencilBits)
{
    if (depthBits > 0 || stencilBits > 0)
    {
        if (stencilBits == 0)
        {
            if (depthBits <= 16)
                return WGPUTextureFormat_Depth16Unorm;
            else if (depthBits <= 24)
                return WGPUTextureFormat_Depth24Plus;
            else
                return WGPUTextureFormat_Depth32Float;
        }
        else
        {
            if (depthBits == 0)
                return WGPUTextureFormat_Stencil8;
            else if (depthBits <= 24)
                return WGPUTextureFormat_Depth24PlusStencil8;
            else
                return WGPUTextureFormat_Depth32FloatStencil8;
        }
    }
    return WGPUTextureFormat_Undefined;
}

WGSwapChain::WGSwapChain(WGRenderSystem& renderSystem, const SwapChainDescriptor& desc, const std::shared_ptr<Surface>& surface) :
    device_             { renderSystem.GetNativeDevice()                           },
    depthStencilFormat_ { PickDepthStencilFormat(desc.depthBits, desc.stencilBits) }
{
    /* Setup surface for the swap-chain */
    SetOrCreateSurface(surface, SwapChain::BuildDefaultSurfaceTitle(renderSystem.GetRendererInfo()), desc);
    CreateWebGpuSurface(renderSystem.GetNativeInstance(), desc, GetSurface());
    UpdateWebGpuSurface(desc.resolution, WGPUPresentMode_Mailbox);
}

WGSwapChain::~WGSwapChain()
{
    if (surface_ != nullptr)
    {
        wgpuSurfaceRelease(surface_);
        surface_ = nullptr;
    }
    ReleaseTransientFramebuffer();
    ReleaseDepthStencilTexture();
}

bool WGSwapChain::IsPresentable() const
{
    return true; // dummy
}

void WGSwapChain::Present()
{
    if (framebuffer_.colorTexture != nullptr)
    {
        /* Present surface and release transient framebuffer targets  */
        wgpuSurfacePresent(surface_);
        ReleaseTransientFramebuffer();
    }
}

std::uint32_t WGSwapChain::GetCurrentSwapIndex() const
{
    return 0; //todo
}

std::uint32_t WGSwapChain::GetNumSwapBuffers() const
{
    return 1; //todo
}

std::uint32_t WGSwapChain::GetSamples() const
{
    return 1; //todo
}

Format WGSwapChain::GetColorFormat() const
{
    return Format::BGRA8UNorm; //todo
}

Format WGSwapChain::GetDepthStencilFormat() const
{
    return Format::D24UNormS8UInt; //todo
}

const RenderPass* WGSwapChain::GetRenderPass() const
{
    return nullptr; //todo
}

bool WGSwapChain::SetVsyncInterval(std::uint32_t vsyncInterval)
{
    const bool isVsyncEnabled = (vsyncInterval != 0);
    UpdateWebGpuSurface(resolution_, (isVsyncEnabled ? WGPUPresentMode_Fifo : WGPUPresentMode_Mailbox));
    return true;
}

WGFramebuffer WGSwapChain::GetCurrentFramebuffer()
{
    if (framebuffer_.colorTexture == nullptr)
    {
        WGPUSurfaceTexture surfaceTexture = {};
        wgpuSurfaceGetCurrentTexture(surface_, &surfaceTexture);

        if (surfaceTexture.status != WGPUSurfaceGetCurrentTextureStatus_SuccessOptimal &&
            surfaceTexture.status != WGPUSurfaceGetCurrentTextureStatus_SuccessSuboptimal)
        {
            // Handle error (e.g., window resized or lost)
            return {};
        }

        framebuffer_.colorTexture       = surfaceTexture.texture;
        framebuffer_.colorTextureView   = wgpuTextureCreateView(surfaceTexture.texture, NULL);
    }
    return framebuffer_;
}


/*
 * ======= Private: =======
 */

bool WGSwapChain::ResizeBuffersPrimary(const Extent2D& resolution)
{
    UpdateWebGpuSurface(resolution, presentMode_);
    return true;
}

void WGSwapChain::CreateWebGpuSurface(WGPUInstance instance, const SwapChainDescriptor& desc, Surface& surface)
{
    NativeHandle hantiveHandle;
    surface.GetNativeHandle(&hantiveHandle, sizeof(hantiveHandle));

    #if defined(LLGL_OS_WIN32)

    WGPUSurfaceSourceWindowsHWND windowDesc;
    {
        windowDesc.chain        = { nullptr, WGPUSType_SurfaceSourceWindowsHWND },
        windowDesc.hinstance    = GetModuleHandle(NULL);
        windowDesc.hwnd         = hantiveHandle.window;
    };

    #else

    #   error Platform not supported for WebGPU surface creation

    #endif

    WGPUSurfaceDescriptor surfaceDesc = { &(windowDesc.chain), WGPU_STRING_VIEW_INIT };

    surface_ = wgpuInstanceCreateSurface(instance, &surfaceDesc);
    LLGL_ASSERT_PTR(surface_);
}

void WGSwapChain::UpdateWebGpuSurface(const Extent2D& resolution, WGPUPresentMode presentMode)
{
    if (presentMode_ != presentMode || resolution_ != resolution)
    {
        /* Cache new configuration */
        presentMode_ = presentMode;
        resolution_ = resolution;

        WGPUSurfaceConfiguration config;
        {
            config.nextInChain      = nullptr;
            config.device           = device_;
            config.format           = colorFormat_;
            config.usage            = WGPUTextureUsage_RenderAttachment;
            config.width            = resolution.width;
            config.height           = resolution.height;
            config.viewFormatCount  = 0;
            config.viewFormats      = nullptr;
            config.alphaMode        = WGPUCompositeAlphaMode_Opaque;
            config.presentMode      = presentMode;
        }
        wgpuSurfaceConfigure(surface_, &config);

        /* Create depth-stencil texture if enabled */
        if (depthStencilFormat_ != WGPUTextureFormat_Undefined)
            CreateDepthStencilTexture(resolution);
    }
}

void WGSwapChain::ReleaseTransientFramebuffer()
{
    if (framebuffer_.colorTextureView != nullptr)
    {
        wgpuTextureViewRelease(framebuffer_.colorTextureView);
        framebuffer_.colorTextureView = nullptr;
    }
    if (framebuffer_.colorTexture != nullptr)
    {
        wgpuTextureRelease(framebuffer_.colorTexture);
        framebuffer_.colorTexture = nullptr;
    }
}

void WGSwapChain::CreateDepthStencilTexture(const Extent2D& resolution)
{
    /* Create WebGPU texture for depth-stencil */
    WGPUTextureDescriptor depthStencilDesc;
    {
        depthStencilDesc.nextInChain                = nullptr;
        depthStencilDesc.label                      = WGPU_STRING_VIEW_INIT;
        depthStencilDesc.usage                      = WGPUTextureUsage_RenderAttachment;
        depthStencilDesc.dimension                  = WGPUTextureDimension_2D;
        depthStencilDesc.size.width                 = resolution.width;
        depthStencilDesc.size.height                = resolution.height;
        depthStencilDesc.size.depthOrArrayLayers    = 1;
        depthStencilDesc.format                     = depthStencilFormat_;
        depthStencilDesc.mipLevelCount              = 1;
        depthStencilDesc.sampleCount                = 1;
        depthStencilDesc.viewFormatCount            = 0;
        depthStencilDesc.viewFormats                = nullptr; //???
    }
    framebuffer_.depthStencil = wgpuDeviceCreateTexture(device_, &depthStencilDesc);
    WGThrowIfCreateFailed(framebuffer_.depthStencil, "WGPUTexture");

    /* Create WebGPU texture view for depth-stencil */
    WGPUTextureViewDescriptor depthStencilViewDesc;
    {
        depthStencilViewDesc.nextInChain        = nullptr;
        depthStencilViewDesc.label              = WGPU_STRING_VIEW_INIT;
        depthStencilViewDesc.format             = depthStencilFormat_;
        depthStencilViewDesc.dimension          = WGPUTextureViewDimension_2D;
        depthStencilViewDesc.baseMipLevel       = 0;
        depthStencilViewDesc.mipLevelCount      = 1;
        depthStencilViewDesc.baseArrayLayer     = 0;
        depthStencilViewDesc.arrayLayerCount    = 1;
        depthStencilViewDesc.aspect             = WGPUTextureAspect_All;
        depthStencilViewDesc.usage              = WGPUTextureUsage_RenderAttachment;
    }
    framebuffer_.depthStencilView = wgpuTextureCreateView(framebuffer_.depthStencil, &depthStencilViewDesc);
    WGThrowIfCreateFailed(framebuffer_.depthStencilView, "WGPUTextureView");
}

void WGSwapChain::ReleaseDepthStencilTexture()
{
    if (framebuffer_.depthStencilView != nullptr)
    {
        wgpuTextureViewRelease(framebuffer_.depthStencilView);
        framebuffer_.depthStencilView = nullptr;
    }
    if (framebuffer_.depthStencil != nullptr)
    {
        wgpuTextureRelease(framebuffer_.depthStencil);
        framebuffer_.depthStencil = nullptr;
    }
}


} // /namespace LLGL



// ================================================================================
