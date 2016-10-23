/*
 * RenderContext.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/RenderContext.h>


namespace LLGL
{


RenderContext::~RenderContext()
{
}

void RenderContext::SetVideoMode(const VideoModeDescriptor& videoModeDesc)
{
    if (videoModeDesc_ != videoModeDesc)
    {
        /* Update window appearance */
        auto windowDesc = GetWindow().QueryDesc();

        windowDesc.size = videoModeDesc.resolution;

        if (videoModeDesc.fullscreen)
        {
            windowDesc.borderless   = true;
            windowDesc.position     = { 0, 0 };
        }
        else
        {
            windowDesc.borderless   = false;
            windowDesc.centered     = true;
        }

        GetWindow().SetDesc(windowDesc);

        /* Store new video mode */
        videoModeDesc_ = videoModeDesc;
    }
}


/*
 * ======= Protected: =======
 */

void RenderContext::SetOrCreateWindow(const std::shared_ptr<Window>& window, VideoModeDescriptor& videoModeDesc, const void* windowContext)
{
    if (!window)
    {
        /* Create new window for this render context */
        WindowDescriptor windowDesc;
        {
            windowDesc.size             = videoModeDesc.resolution;
            windowDesc.borderless       = videoModeDesc.fullscreen;
            windowDesc.centered         = !videoModeDesc.fullscreen;
            windowDesc.windowContext    = windowContext;
        }
        window_ = Window::Create(windowDesc);
    }
    else
    {
        /* Get and output resolution from specified window */
        videoModeDesc.resolution = window->GetSize();
        window_ = window;
    }

    /* Store video mode settings */
    videoModeDesc_ = videoModeDesc;
}

void RenderContext::ShareWindowAndVideoMode(RenderContext& other)
{
    window_         = other.window_;
    videoModeDesc_  = other.videoModeDesc_;
}


} // /namespace LLGL



// ================================================================================
