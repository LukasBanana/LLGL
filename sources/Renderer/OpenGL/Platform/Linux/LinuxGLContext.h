/*
 * LinuxGLContext.h
 *
 * Copyright (c) 2025 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_LINUX_GL_CONTEXT_H
#define LLGL_LINUX_GL_CONTEXT_H

#include "../GLContext.h"


namespace LLGL
{

// GNU/Linux specific abstraction over the <GLContext> interface for GNU/Linux.
class LinuxGLContext : public GLContext
{

public:
    virtual bool IsWayland() const = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
