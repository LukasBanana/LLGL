/*
 * Desktop.cpp (IOS)
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#import <UIKit/UIKit.h>

#include <LLGL/Desktop.h>


namespace LLGL
{

namespace Desktop
{


LLGL_EXPORT Extent2D GetResolution()
{
    /* Get pixel size from main display */
    CGRect screenRect = [[UIScreen mainScreen] bounds];
    return
    {
        static_cast<std::uint32_t>(screenRect.size.width),
        static_cast<std::uint32_t>(screenRect.size.height)
    };
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
