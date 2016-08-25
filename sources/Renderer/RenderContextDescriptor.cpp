/*
 * RenderContextDescriptor.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/RenderContextDescriptor.h>


namespace LLGL
{


LLGL_EXPORT bool operator == (const VideoModeDescriptor& lhs, const VideoModeDescriptor& rhs)
{
    return
        lhs.resolution.x == rhs.resolution.x &&
        lhs.resolution.y == rhs.resolution.y &&
        lhs.colorDepth == rhs.colorDepth &&
        lhs.fullscreen == rhs.fullscreen;
}

LLGL_EXPORT bool operator != (const VideoModeDescriptor& lhs, const VideoModeDescriptor& rhs)
{
    return !(lhs == rhs);
}


} // /namespace LLGL



// ================================================================================
