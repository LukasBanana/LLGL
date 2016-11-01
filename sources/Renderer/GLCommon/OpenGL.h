/*
 * OpenGL.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_OPENGL_H
#define LLGL_OPENGL_H


#include <LLGL/Platform/Platform.h>

#if defined(LLGL_OS_WIN32)
#   include <Windows.h>
#   include <gl/GL.h>
#   include <GL/glext.h>
#   include <GL/wglext.h>
#elif defined(LLGL_OS_LINUX)
#   include <GL/gl.h>
#   include <GL/glext.h>
#   include <GL/glx.h>
#elif defined(LLGL_OS_MACOS)
#   include <OpenGL/gl3.h>
#   include <OpenGL/glext.h>
#   include "Platform/MacOS/MacOSGLExt.h"
#elif defined(LLGL_OS_IOS)
#   include <OpenGLES/ES2/gl.h>
#   include <OpenGLES/ES2/glext.h>
#elif defined(LLGL_OS_ANDROID)
#   include <GLES2/gl2.h>
#   include <GLES2/gl2ext.h>
#endif


#endif



// ================================================================================
