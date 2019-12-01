/*
 * OpenGLES.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
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
