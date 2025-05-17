/*
 * LinuxNativeHandle.h (OpenGL)
 * 
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_OPENGL_LINUX_NATIVE_HANDLE_H
#define LLGL_OPENGL_LINUX_NATIVE_HANDLE_H


#include <LLGL/Deprecated.h>
#include <GL/glx.h>

#if LLGL_EXPOSE_WAYLAND || LLGL_LINUX_ENABLE_WAYLAND
    #include <EGL/egl.h>
#endif


namespace LLGL
{

namespace OpenGL
{

enum class RenderSystemNativeHandleType : char {
    GLX = 0,
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
        LLGL_DEPRECATED("Deprecated since 0.04b; Use glx instead.")
        GLXContext context;
        GLXContext glx;
        #if LLGL_EXPOSE_WAYLAND || LLGL_LINUX_ENABLE_WAYLAND
        EGLContext egl;
        #else
        void*      egl;
        #endif
    };

    RenderSystemNativeHandleType type;
};


} // /namespace OpenGL

} // /namespace LLGL


#endif



// ================================================================================
