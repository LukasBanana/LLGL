/*
 * OpenGLES2.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_OPENGLES2_H
#define LLGL_OPENGLES2_H


#include <LLGL/Platform/Platform.h>

#if defined(LLGL_OS_IOS)
#   include <OpenGLES/ES2/gl.h>
#   include <OpenGLES/ES2/glext.h>
#elif defined(LLGL_OS_ANDROID)
#   include <GLES2/gl2.h>
#   include <GLES2/gl2ext.h>
#else
#   error "Unsupported platform for OpenGL ES 2"
#endif


#endif



// ================================================================================
