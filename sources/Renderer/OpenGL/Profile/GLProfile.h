/*
 * GLProfile.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_GL_PROFILE_H
#define LLGL_GL_PROFILE_H


#include <LLGL/RendererConfiguration.h>

#if LLGL_OPENGL
#   if LLGL_GL_ENABLE_OPENGL2X
#       include "GLCompat/GLCompatProfileTypes.h"
#   else
#       include "GLCore/GLCoreProfileTypes.h"
#   endif
#elif LLGL_OPENGLES3
#   include "GLES/GLESProfileTypes.h"
#elif LLGL_WEBGL
#   include "WebGL/WebGLProfileTypes.h"
#else
#   error Unknwon OpenGL backend
#endif


namespace LLGL
{

// Wrapper namespace to abstract OpenGL and OpenGLES API calls
namespace GLProfile
{


// Returns the renderer ID number, e.g. RendererID::OpenGL or RendererID::OpenGLES.
int GetRendererID();

// Returns the renderer module name, e.g. "OpenGL" or "OpenGLES3".
const char* GetModuleName();

// Returns the renderer name, e.g. "OpenGL Core", "OpenGL Compatibility", "OpenGL ES 3" etc..
const char* GetRendererName();

// Returns the OpenGL API name without version number, e.g. "OpenGL" or "OpenGL ES".
const char* GetAPIName();

// Returns the OpenGL shading language name, e.g. "GLSL" or "ESSL".
const char* GetShadingLanguageName();

// Returns the preferred GL context profile.
OpenGLContextProfile GetContextProfile();

// Returns the maximum number of viewports (GL_MAX_VIEWPORT for GL, 1 for GLES).
GLint GetMaxViewports();

// Wrapper for glDepthRange/glDepthRangef.
void DepthRange(GLclamp_t nearVal, GLclamp_t farVal);

// Wrapper for glClearDepth/glClearDepthf.
void ClearDepth(GLclamp_t depth);

// Wrapper for glBufferSubData; uses glMapBufferRange for GLES.
void GetBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, void* data);

// Wrapper for glMapBuffer; uses glMapBufferRange for GLES.
void* MapBuffer(GLenum target, GLenum access);

// Wrapper for glMapBufferRange; uses glMapBuffer for GL.
void* MapBufferRange(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access);

// Wrapper for glUnmapBuffer. Not supported in WebGL.
void UnmapBuffer(GLenum target);

// Wrapper for glDrawBuffer; uses glDrawBuffers for GLES.
void DrawBuffer(GLenum buf);

// Wrapper functions for glFramebufferTexture* for GL and GLES.
void FramebufferTexture1D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
void FramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
void FramebufferTexture3D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint layer);
void FramebufferTextureLayer(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer);


} // /namespace GLProfile

} // /namespace LLGL


#endif



// ================================================================================
