/*
 * RenderContext.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/RenderContext.h>
#include <LLGL/Window.h>
#include <LLGL/Canvas.h>
#include "CheckedCast.h"


namespace LLGL
{


RenderContext::~RenderContext()
{
}

void RenderContext::SetVideoMode(const VideoModeDescriptor& videoModeDesc)
{
    if (videoModeDesc_ != videoModeDesc)
    {
        /* Store new video mode */
        videoModeDesc_ = videoModeDesc;

        /* Adapt surface for the new video mode */
        GetSurface().AdaptForVideoMode(videoModeDesc_);
    }
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

void RenderContext::ShareSurfaceAndVideoMode(RenderContext& other)
{
    surface_        = other.surface_;
    videoModeDesc_  = other.videoModeDesc_;
}


} // /namespace LLGL



// ================================================================================
