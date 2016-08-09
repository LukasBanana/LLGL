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


/*
 * ======= Protected: =======
 */

void RenderContext::SetWindow(const std::shared_ptr<Window>& window, VideoModeDescriptor& videoModeDesc, const void* windowContext)
{
    window_ = window;

    if (!window_)
    {
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
        videoModeDesc.resolution = window_->GetSize();
}


} // /namespace LLGL



// ================================================================================
