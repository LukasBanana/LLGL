/*
 * Desktop.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/Desktop.h>
#include <X11/Xlib.h>


namespace LLGL
{

namespace Desktop
{


LLGL_EXPORT Extent2D GetResolution()
{
    if (auto dpy = XOpenDisplay(nullptr))
    {
        if (auto scr = DefaultScreenOfDisplay(dpy))
        {
            return
            {
                static_cast<std::uint32_t>(scr->width),
                static_cast<std::uint32_t>(scr->height)
            };
        }
        XCloseDisplay(dpy);
    }
    return { 0u, 0u };
}

LLGL_EXPORT int GetColorDepth()
{
    return 24;
}

LLGL_EXPORT bool SetVideoMode(const VideoModeDescriptor& videoMode)
{
    return false;
}

LLGL_EXPORT bool ResetVideoMode()
{
    return false;
}


} // /namespace Desktop

} // /namespace LLGL



// ================================================================================
