/*
 * Win32GLPlatformContext.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_WIN32_GL_PLATFORM_CONTEXT_H__
#define __LLGL_WIN32_GL_PLATFORM_CONTEXT_H__


#include "../OpenGL.h"
#include <vector>


namespace LLGL
{


struct GLPlatformContext
{
    int                 pixelFormat     = 0;    //!< Standard pixel format.
    std::vector<int>    msPixelFormats;         //!< Multi-sampled pixel formats.

    HDC                 hDC             = 0;    //!< Device context handle.
    HGLRC               hGLRC           = 0;    //!< OpenGL render context handle.
};


} // /namespace LLGL


#endif



// ================================================================================
