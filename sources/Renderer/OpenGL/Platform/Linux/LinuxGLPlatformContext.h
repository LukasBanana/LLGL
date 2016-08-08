/*
 * LinuxGLPlatformContext.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_LINUX_GL_PLATFORM_CONTEXT_H__
#define __LLGL_LINUX_GL_PLATFORM_CONTEXT_H__


#include "../../OpenGL.h"
#include <X11/Xlib.h>


namespace LLGL
{


struct GLPlatformContext
{
    ::Display*      display;
    ::Window        wnd;
    XVisualInfo*    visual;
    GLXContext      glc;
};


} // /namespace LLGL


#endif



// ================================================================================
