/*
 * Desktop.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/Desktop.h>


namespace LLGL
{

namespace Desktop
{


LLGL_EXPORT Size GetResolution()
{
    return { 1920, 1080 };//!!!
}

LLGL_EXPORT int GetColorDepth()
{
    return 24;//!!!
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
