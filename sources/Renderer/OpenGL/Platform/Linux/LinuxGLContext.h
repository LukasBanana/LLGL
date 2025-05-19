/*
 * LinuxGLContext.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_LINUX_GL_CONTEXT_H
#define LLGL_LINUX_GL_CONTEXT_H


#include "../GLContext.h"
#include <LLGL/Backend/OpenGL/NativeHandle.h>


namespace LLGL
{


// GNU/Linux specific abstraction over the <GLContext> interface for GNU/Linux.
class LinuxGLContext : public GLContext
{

    public:

        // Returns the native type of this GL context (GLX or EGL).
        virtual OpenGL::RenderSystemNativeType GetNativeType() const = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
