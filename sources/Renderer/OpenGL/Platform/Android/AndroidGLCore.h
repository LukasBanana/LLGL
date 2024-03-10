/*
 * AndroidGLCore.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_ANDROID_GL_CORE_H
#define LLGL_ANDROID_GL_CORE_H


#include <EGL/egl.h>


namespace LLGL
{


// Returns a string representation of the specified EGL error code or an empty string if the error code is invalid.
const char* EGLErrorToString(EGLint errorCode);

// Calls eglGetError() on the primary version of EGLErrorToString().
const char* EGLErrorToString();


} // /namespace LLGL


#endif



// ================================================================================
