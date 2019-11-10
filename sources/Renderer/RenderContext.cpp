/*
 * RenderContext.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/RenderContext.h>
#include <LLGL/Window.h>
#include <LLGL/Canvas.h>
#include <LLGL/Display.h>
#include "CheckedCast.h"
#include "../Core/Helper.h"


namespace LLGL
{


RenderContext::RenderContext(const VideoModeDescriptor& initialVideoMode, const VsyncDescriptor& initialVsync) :
    videoModeDesc_ { initialVideoMode },
    vsyncDesc_     { initialVsync     }
{
}

/* ----- Render Target ----- */

Extent2D RenderContext::GetResolution() const
{
    return GetVideoMode().resolution;
}

std::uint32_t RenderContext::GetNumColorAttachments() const
{
    return 1u;
}

bool RenderContext::HasDepthAttachment() const
{
    return IsDepthFormat(GetDepthStencilFormat());
}

bool RenderContext::HasStencilAttachment() const
{
    return IsStencilFormat(GetDepthStencilFormat());
}

/* ----- Configuration ----- */

static bool IsVideoModeValid(const VideoModeDescriptor& videoModeDesc)
{
    return (videoModeDesc.resolution.width > 0 && videoModeDesc.resolution.height > 0 && videoModeDesc.swapChainSize > 0);
}

bool RenderContext::SetVideoMode(const VideoModeDescriptor& videoModeDesc)
{
    if (IsVideoModeValid(videoModeDesc))
    {
        if (videoModeDesc_ != videoModeDesc)
            return SetVideoModePrimary(videoModeDesc);
        else
            return true;
    }
    return false;
}

bool RenderContext::SetVsync(const VsyncDescriptor& vsyncDesc)
{
    if (vsyncDesc_ != vsyncDesc)
    {
        /* Call primary V-sync function */
        if (OnSetVsync(vsyncDesc))
        {
            vsyncDesc_ = vsyncDesc;
            return true;
        }
        return false;
    }
    return true;
}


/*
 * ======= Protected: =======
 */

void RenderContext::SetOrCreateSurface(const std::shared_ptr<Surface>& surface, VideoModeDescriptor videoModeDesc, const void* windowContext)
{
    if (surface)
    {
        /* Get and output resolution from specified window */
        videoModeDesc.resolution = surface->GetContentSize();
        surface_ = surface;
    }
    else
    {
        #ifdef LLGL_MOBILE_PLATFORM

        /* Create new canvas for this render context */
        CanvasDescriptor canvasDesc;
        {
            canvasDesc.borderless = videoModeDesc.fullscreen;
        }
        surface_ = Canvas::Create(canvasDesc);

        #else

        /* Create new window for this render context */
        WindowDescriptor windowDesc;
        {
            windowDesc.size             = videoModeDesc.resolution;
            windowDesc.borderless       = videoModeDesc.fullscreen;
            windowDesc.centered         = !videoModeDesc.fullscreen;
            windowDesc.windowContext    = windowContext;
        }
        surface_ = Window::Create(windowDesc);

        #endif
    }

    /* Store video mode settings */
    videoModeDesc_ = videoModeDesc;

    /* Switch to fullscreen mode before storing new video mode */
    if (videoModeDesc_.fullscreen)
        SetDisplayMode(videoModeDesc_);
}

void RenderContext::ShareSurfaceAndConfig(RenderContext& other)
{
    surface_        = other.surface_;
    videoModeDesc_  = other.videoModeDesc_;
    vsyncDesc_      = other.vsyncDesc_;
}

bool RenderContext::SetDisplayMode(const VideoModeDescriptor& videoModeDesc)
{
    if (surface_)
    {
        if (auto display = surface_->FindResidentDisplay())
        {
            if (videoModeDesc.fullscreen)
            {
                /* Change display mode resolution to video mode setting */
                auto displayModeDesc = display->GetDisplayMode();
                displayModeDesc.resolution = videoModeDesc.resolution;
                return display->SetDisplayMode(displayModeDesc);
            }
            else
            {
                /* Reset display mode to default */
                return display->ResetDisplayMode();
            }
        }
    }
    return false;
}

bool RenderContext::SetDisplayFullscreenMode(const VideoModeDescriptor& videoModeDesc)
{
    if (GetVideoMode().fullscreen != videoModeDesc.fullscreen)
        return SetDisplayMode(videoModeDesc);
    else
        return true;
}


/*
 * ======= Private: =======
 */

bool RenderContext::OnIsRenderContext() const
{
    return true;
}

bool RenderContext::SetVideoModePrimary(const VideoModeDescriptor& videoModeDesc)
{
    bool result = true;

    auto& surface = GetSurface();

    /* Store current surface position if the render context is in windowed mode */
    if (!videoModeDesc_.fullscreen && videoModeDesc.fullscreen)
        StoreSurfacePosition();

    /* Adapt surface for the new video mode */
    auto finalVideoMode = videoModeDesc;
    surface.AdaptForVideoMode(finalVideoMode);

    /* Call primary video mode function */
    if (OnSetVideoMode(finalVideoMode))
    {
        /* Store video mode on success */
        videoModeDesc_ = finalVideoMode;
    }
    else
    {
        /* Reset surface for previous video mode */
        surface.AdaptForVideoMode(videoModeDesc_);
        result = false;
    }

    if (!videoModeDesc_.fullscreen)
        RestoreSurfacePosition();

    return result;
}

void RenderContext::StoreSurfacePosition()
{
    #ifndef LLGL_MOBILE_PLATFORM
    auto& window = static_cast<Window&>(GetSurface());
    cachedSurfacePos_ = MakeUnique<Offset2D>(window.GetPosition());
    #endif
}

void RenderContext::RestoreSurfacePosition()
{
    #ifndef LLGL_MOBILE_PLATFORM
    if (cachedSurfacePos_)
    {
        auto& window = static_cast<Window&>(GetSurface());
        window.SetPosition(*cachedSurfacePos_);
        cachedSurfacePos_.reset();
    }
    #endif
}


} // /namespace LLGL



// ================================================================================
