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

RenderContext::RenderContext(const std::shared_ptr<Window>& window, const VideoModeDescriptor& videoModeDesc) :
    window_( window )
{
    if (!window_)
    {
        WindowDescriptor windowDesc;
        {
            windowDesc.size.x       = videoModeDesc.screenWidth;
            windowDesc.size.y       = videoModeDesc.screenHeight;
            windowDesc.visible      = true;
            windowDesc.borderless   = videoModeDesc.fullscreen;
            windowDesc.centered     = !videoModeDesc.fullscreen;
        }
        window_ = Window::Create(windowDesc);
    }
}


} // /namespace LLGL



// ================================================================================
