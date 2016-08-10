/*
 * OpenGL.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_OPENGL_H__
#define __LLGL_OPENGL_H__


#if defined(WIN32)
#   include <Windows.h>
#   include <gl/GL.h>
#   include <GL/glext.h>
#   include <GL/wglext.h>
#elif defined(__linux__)
#   include <GL/gl.h>
#   include <GL/glext.h>
#   include <GL/glx.h>
#elif defined(__APPLE__)
#   include <OpenGL/gl3.h>
#   include <OpenGL/glext.h>
#   define GL_DEBUG_OUTPUT                  0
#   define GL_DEBUG_OUTPUT_SYNCHRONOUS      0
#   define GL_PRIMITIVE_RESTART_FIXED_INDEX 0
#   define GL_ATOMIC_COUNTER_BUFFER         0
#   define GL_DISPATCH_INDIRECT_BUFFER      0
#   define GL_QUERY_BUFFER                  0
#   define GL_SHADER_STORAGE_BUFFER         0
#   define GL_COMPUTE_SHADER                0
#endif


#endif



// ================================================================================
