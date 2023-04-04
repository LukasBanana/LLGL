/*
 * OpenGLES.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_OPENGLES_H
#define LLGL_OPENGLES_H


#include <LLGL/Platform/Platform.h>

#if defined(LLGL_OS_IOS)
#   include <OpenGLES/ES3/gl.h>
#   include <OpenGLES/ES3/glext.h>
#elif defined(LLGL_OS_ANDROID)
#   include <GLES3/gl3.h>
#   include <GLES3/gl3ext.h>
#else
#   error Unsupported platform for OpenGLES 3
#endif


#endif



// ================================================================================
