/*
 * OpenGLCore.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_OPENGL_CORE_H
#define LLGL_OPENGL_CORE_H


#include <LLGL/Platform/Platform.h>

#if defined LLGL_OS_WIN32
#   include "../../../../Platform/Win32/Win32LeanAndMean.h"
#   include <Windows.h>
#   include <GL/GL.h>
#   include <GL/glext.h>
#   include <GL/wglext.h>
#elif defined LLGL_OS_LINUX
#   include <GL/gl.h>
#   include <GL/glext.h>
#   include <GL/glx.h>
#elif defined LLGL_OS_MACOS
#   include "../../../../Platform/MacOS/MacOSCompatibility.h"
#   if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_7
#       include <OpenGL/gl3.h>
#   else
#       include <OpenGL/gl.h>
#   endif
#   include <OpenGL/glext.h>
#   include "../../Platform/MacOS/MacOSGLExt.h"
#else
#   error Unsupported platform for OpenGL
#endif


#endif



// ================================================================================
