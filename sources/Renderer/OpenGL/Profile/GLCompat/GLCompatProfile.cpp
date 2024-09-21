/*
 * GLCompatProfile.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "../GLProfile.h"
#include "../../../../Core/Exception.h"
#include "GLCompatExtensions.h"
#include <LLGL/RenderSystemFlags.h>


namespace LLGL
{

namespace GLProfile
{


int GetRendererID()
{
    return RendererID::OpenGL;
}

const char* GetModuleName()
{
    return "OpenGL";
}

const char* GetRendererName()
{
    return "OpenGL Compatibility";
}

const char* GetAPIName()
{
    return "OpenGL";
}

const char* GetShadingLanguageName()
{
    return "GLSL";
}

OpenGLContextProfile GetContextProfile()
{
    return OpenGLContextProfile::CompatibilityProfile;
}

GLint GetMaxViewports()
{
    return 1;
}

void DepthRange(GLclamp_t nearVal, GLclamp_t farVal)
{
    glDepthRange(nearVal, farVal);
}

void ClearDepth(GLclamp_t depth)
{
    glClearDepth(depth);
}

void GetBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, void* data)
{
    glGetBufferSubData(target, offset, size, data);
}

void* MapBuffer(GLenum target, GLenum access)
{
    return glMapBuffer(target, access);
}

static GLenum ToGLMapBufferRangeAccess(GLbitfield access)
{
    if ((access & (GL_MAP_READ_BIT | GL_MAP_WRITE_BIT)) == (GL_MAP_READ_BIT | GL_MAP_WRITE_BIT))
        return GL_READ_WRITE;
    if ((access & GL_MAP_READ_BIT) != 0)
        return GL_READ_ONLY;
    if ((access & GL_MAP_WRITE_BIT) != 0)
        return GL_WRITE_ONLY;
    return 0;
}

void* MapBufferRange(GLenum target, GLintptr offset, GLsizeiptr /*length*/, GLbitfield access)
{
    char* ptr = reinterpret_cast<char*>(glMapBuffer(target, ToGLMapBufferRangeAccess(access)));
    return reinterpret_cast<void*>(ptr + offset);
}

void UnmapBuffer(GLenum target)
{
    glUnmapBuffer(target);
}

void DrawBuffer(GLenum buf)
{
    glDrawBuffer(buf);
}

void FramebufferTexture1D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
    glFramebufferTexture1D(target, attachment, textarget, texture, level);
}

void FramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
    glFramebufferTexture2D(target, attachment, textarget, texture, level);
}

void FramebufferTexture3D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLint layer)
{
    glFramebufferTexture3D(target, attachment, textarget, texture, level, layer);
}

void FramebufferTextureLayer(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer)
{
    LLGL_TRAP_FEATURE_NOT_SUPPORTED("glFramebufferTextureLayer");
}


} // /namespace GLProfile

} // /namespace LLGL



// ================================================================================
