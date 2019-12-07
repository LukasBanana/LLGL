/*
 * GLProfile.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_PROFILE_H
#define LLGL_GL_PROFILE_H


#if defined LLGL_OPENGL
#   include "GLCoreProfile/GLCoreProfileTypes.h"
#elif defined LLGL_OPENGLES3
#   include "GLESProfile/GLESProfileTypes.h"
#endif


namespace LLGL
{

// Wrapper namespace to abstract OpenGL and OpenGLES API calls
namespace GLProfile
{


// Returns the renderer ID number, e.g. RendererID::OpenGL or RendererID::OpenGLES3.
int GetRendererID();

// Returns the renderer module name, e.g. "OpenGL" or "OpenGLES3".
const char* GetModuleName();

// Returns the renderer name, e.g. "OpenGL" or "OpenGL ES 3".
const char* GetRendererName();

// Returns the OpenGL API name without version number, e.g. "OpenGL" or "OpenGL ES".
const char* GetAPIName();

// Returns the OpenGL shading language name, e.g. "GLSL" or "ESSL".
const char* GetShadingLanguageName();

// Returns the maximum number of viewports (GL_MAX_VIEWPORT for GL, 1 for GLES).
GLint GetMaxViewports();

// Returns the internal format of the first texture level for the specified bound texture target.
void GetTexParameterInternalFormat(GLenum target, GLint* params);

// Wrapper for glGetInternalformativ with special case for GLES.
void GetInternalformativ(GLenum target, GLenum internalformat, GLenum pname, GLsizei bufsize, GLint* params);

// Wrapper for glDepthRange/glDepthRangef.
void DepthRange(GLclamp_t nearVal, GLclamp_t farVal);

// Wrapper for glClearDepth/glClearDepthf.
void ClearDepth(GLclamp_t depth);

// Wrapper for glBufferSubData; uses glMapBufferRange for GLES.
void GetBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, void* data);

// Wrapper for glMapBuffer; uses glMapBufferRange for GLES.
void* MapBuffer(GLenum target, GLenum access);

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
