/*
 * SwapChain.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/SwapChain.h>
#include <LLGL/Window.h>
#include <LLGL/Canvas.h>
#include <LLGL/Display.h>
#include "CheckedCast.h"
#include "../Core/CoreUtils.h"


namespace LLGL
{


struct SwapChain::Pimpl
{
    std::shared_ptr<Surface>    surface;
    Extent2D                    resolution;
    Offset2D                    normalModeSurfacePos;
    bool                        normalModeSurfacePosStored = false;
};

SwapChain::SwapChain() :
    pimpl_ { new Pimpl{} }
{
}

SwapChain::SwapChain(const SwapChainDescriptor& desc) :
    SwapChain {}
{
    pimpl_->resolution = desc.resolution;
}

SwapChain::~SwapChain()
{
    delete pimpl_;
}

/* ----- Render Target ----- */

Extent2D SwapChain::GetResolution() const
{
    return pimpl_->resolution;
}

std::uint32_t SwapChain::GetNumColorAttachments() const
{
    return 1u;
}

bool SwapChain::HasDepthAttachment() const
{
    return IsDepthFormat(GetDepthStencilFormat());
}

bool SwapChain::HasStencilAttachment() const
{
    return IsStencilFormat(GetDepthStencilFormat());
}

bool SwapChain::ResizeBuffers(const Extent2D& resolution, long flags)
{
    const bool toggleFullscreen = ((flags & (ResizeBuffersFlags::FullscreenMode | ResizeBuffersFlags::WindowedMode)) != 0);
    const bool adaptSurface     = (toggleFullscreen || (flags & ResizeBuffersFlags::AdaptSurface) != 0);

    if (adaptSurface)
    {
        /* Reset fullscreen mode or store surface position for windowed mode */
        bool fullscreen = ((flags & ResizeBuffersFlags::FullscreenMode) != 0);

        if (toggleFullscreen)
        {
            if (fullscreen)
                StoreSurfacePosition();
            else
                ResetDisplayFullscreenMode();
        }

        /* Adapt surface for new resolution */
        auto size = resolution;
        if (GetSurface().AdaptForVideoMode(&size, (toggleFullscreen ? &fullscreen : nullptr)))
        {
            if (ResizeBuffersPrimary(size))
            {
                pimpl_->resolution = size;
                return true;
            }
        }

        /* Switch to fullscreen or restore surface position for windowed mode */
        if (toggleFullscreen)
        {
            if (fullscreen)
                SetDisplayFullscreenMode(size);
            else
                RestoreSurfacePosition();
        }
    }
    else
    {
        /* Only resize swap buffers */
        if (ResizeBuffersPrimary(resolution))
        {
            pimpl_->resolution = resolution;
            return true;
        }
    }

    return false;
}

/* ----- Configuration ----- */

bool SwapChain::SwitchFullscreen(bool enable)
{
    bool result = false;
    if (enable)
    {
        StoreSurfacePosition();
        GetSurface().AdaptForVideoMode(nullptr, &enable);
        result = SetDisplayFullscreenMode(GetResolution());
    }
    else
    {
        result = ResetDisplayFullscreenMode();
        GetSurface().AdaptForVideoMode(nullptr, &enable);
        RestoreSurfacePosition();
    }
    return result;
}

Surface& SwapChain::GetSurface() const
{
    return *(pimpl_->surface);
}


/*
 * ======= Protected: =======
 */

void SwapChain::SetOrCreateSurface(
    const std::shared_ptr<Surface>& surface,
    const UTF8String&               title,
    const Extent2D&                 size,
    bool                            fullscreen,
    const void*                     windowContext,
    std::size_t                     windowContextSize)
{
    if (surface)
    {
        /* Get and output resolution from specified window */
        pimpl_->surface = surface;
    }
    else
    {
        #ifdef LLGL_MOBILE_PLATFORM

        /* Create new canvas for this swap-chain */
        CanvasDescriptor canvasDesc;
        {
            canvasDesc.title = title;
            canvasDesc.flags = (fullscreen ? CanvasFlags::Borderless : 0);
        }
        pimpl_->surface = Canvas::Create(canvasDesc);

        #else // LLGL_MOBILE_PLATFORM

        /* Create new window for this swap-chain */
        WindowDescriptor windowDesc;
        {
            windowDesc.title                = title;
            windowDesc.size                 = size;
            windowDesc.flags                = WindowFlags::DisableSizeScaling | (fullscreen ? WindowFlags::Borderless : WindowFlags::Centered);
            windowDesc.windowContext        = windowContext;
            windowDesc.windowContextSize    = windowContextSize;
        }
        pimpl_->surface = Window::Create(windowDesc);

        #endif // /LLGL_MOBILE_PLATFORM
    }

    /* Take resolution from surface content size */
    pimpl_->resolution = pimpl_->surface->GetContentSize();

    /* Switch to fullscreen mode before storing new video mode */
    if (fullscreen)
        SetDisplayFullscreenMode(pimpl_->resolution);
}

void SwapChain::ShowSurface()
{
    #ifndef LLGL_MOBILE_PLATFORM
    static_cast<Window&>(GetSurface()).Show();
    #endif
}

void SwapChain::ShareSurfaceAndConfig(SwapChain& other)
{
    pimpl_->surface     = other.pimpl_->surface;
    pimpl_->resolution  = other.pimpl_->resolution;
}

bool SwapChain::SetDisplayFullscreenMode(const Extent2D& resolution)
{
    if (auto surface = pimpl_->surface.get())
    {
        if (auto display = surface->FindResidentDisplay())
        {
            /* Change display mode resolution to video mode setting */
            auto displayModeDesc = display->GetDisplayMode();
            displayModeDesc.resolution = resolution;
            return display->SetDisplayMode(displayModeDesc);
        }
    }
    return false;
}

bool SwapChain::ResetDisplayFullscreenMode()
{
    if (auto surface = pimpl_->surface.get())
    {
        if (auto display = surface->FindResidentDisplay())
        {
            /* Reset display mode to default */
            return display->ResetDisplayMode();
        }
    }
    return false;
}

UTF8String SwapChain::BuildDefaultSurfaceTitle(const RendererInfo& info)
{
    UTF8String title = "LLGL";

    /* Append surface typename */
    #ifdef LLGL_MOBILE_PLATFORM
    title += " Canvas ";
    #else
    title += " Window ";
    #endif

    /* Append swap-chain number */
    static int swapChainCounter;
    title += UTF8String{ std::to_string(++swapChainCounter) };

    /* Append renderer name */
    title += " ( ";
    title += UTF8String{ info.rendererName };
    title += " )";

    return title;
}


/*
 * ======= Private: =======
 */

void SwapChain::StoreSurfacePosition()
{
    #ifndef LLGL_MOBILE_PLATFORM
    if (!pimpl_->normalModeSurfacePosStored)
    {
        auto& window = static_cast<Window&>(GetSurface());
        pimpl_->normalModeSurfacePos        = window.GetPosition();
        pimpl_->normalModeSurfacePosStored  = true;
    }
    #endif
}

void SwapChain::RestoreSurfacePosition()
{
    #ifndef LLGL_MOBILE_PLATFORM
    if (pimpl_->normalModeSurfacePosStored)
    {
        auto& window = static_cast<Window&>(GetSurface());
        window.SetPosition(pimpl_->normalModeSurfacePos);
        pimpl_->normalModeSurfacePosStored = false;
    }
    #endif
}


} // /namespace LLGL



// ================================================================================
