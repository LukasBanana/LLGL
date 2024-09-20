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
    // Include all GLES 3.0 functions with static linkage
#   include <GLES3/gl3.h>

    // Include all GLES 3.1+ functions as extensions with dynamic linkage
#   ifdef GL_GLES_PROTOTYPES
#       undef GL_GLES_PROTOTYPES
#       define GL_GLES_PROTOTYPES 0
#   endif
#   if LLGL_GL_ENABLE_OPENGLES == 320
#       include <GLES3/gl32.h>
#   elif LLGL_GL_ENABLE_OPENGLES == 310
#       include <GLES3/gl31.h>
#   endif
#else
#   error Unsupported platform for OpenGLES 3
#endif


#endif



// ================================================================================
