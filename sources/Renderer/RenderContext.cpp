/*
 * RenderContext.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/RenderContext.h>
#include <LLGL/Window.h>
#include <LLGL/Canvas.h>
#include "CheckedCast.h"


namespace LLGL
{


RenderContext::RenderContext(const VsyncDescriptor& initialVsync) :
    vsyncDesc_ { initialVsync }
{
}

static bool IsVideoModeValid(const VideoModeDescriptor& videoModeDesc)
{
    return (videoModeDesc.resolution.width > 0 && videoModeDesc.resolution.height > 0 && videoModeDesc.swapChainSize > 0);
}

bool RenderContext::SetVideoMode(const VideoModeDescriptor& videoModeDesc)
{
    if (IsVideoModeValid(videoModeDesc))
    {
        if (videoModeDesc_ != videoModeDesc)
        {
            /* Adapt surface for the new video mode */
            auto finalVideoMode = videoModeDesc;
            GetSurface().AdaptForVideoMode(finalVideoMode);

            /* Call primary video mode function */
            if (OnSetVideoMode(finalVideoMode))
            {
                /* Store video mode on success */
                videoModeDesc_ = finalVideoMode;
                return true;
            }

            return false;
        }
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

void RenderContext::SetOrCreateSurface(const std::shared_ptr<Surface>& surface, VideoModeDescriptor& videoModeDesc, const void* windowContext)
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
}

void RenderContext::ShareSurfaceAndConfig(RenderContext& other)
{
    surface_        = other.surface_;
    videoModeDesc_  = other.videoModeDesc_;
    vsyncDesc_      = other.vsyncDesc_;
}


} // /namespace LLGL



// ================================================================================
