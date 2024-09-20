/*
 * WebGL.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_WEBGL_H
#define LLGL_WEBGL_H


#include <LLGL/Platform/Platform.h>

#if defined(LLGL_OS_WASM)
#   define GL_GLEXT_PROTOTYPES 1
#   define EGL_EGLEXT_PROTOTYPES 1
#   include <webgl/webgl2.h>
#else
#   error Unsupported platform for WebGL
#endif


#endif



// ================================================================================
