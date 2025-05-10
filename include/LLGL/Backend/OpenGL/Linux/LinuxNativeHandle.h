/*
 * LinuxNativeHandle.h (OpenGL)
 * 
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_OPENGL_LINUX_NATIVE_HANDLE_H
#define LLGL_OPENGL_LINUX_NATIVE_HANDLE_H


#include <GL/glx.h>
#include <EGL/egl.h>


namespace LLGL
{

namespace OpenGL
{

enum class RenderSystemNativeHandleType : char {
    GLX,
    EGL
};

/**
\brief GNU/Linux native handle structure for the OpenGL render system.
\see RenderSystem::GetNativeHandle
\see RenderSystemDescriptor::nativeHandle
*/
struct RenderSystemNativeHandle
{
    union
    {
        GLXContext glx;
        EGLContext egl;
    };

    RenderSystemNativeHandleType type;
};


} // /namespace OpenGL

} // /namespace LLGL


#endif



// ================================================================================
