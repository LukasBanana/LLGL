/*
 * GLCoreProfile.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "../GLProfile.h"
#include "GLCoreExtensions.h"
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
    return "OpenGL";
}

const char* GetAPIName()
{
    return "OpenGL";
}

const char* GetShadingLanguageName()
{
    return "GLSL";
}

GLint GetMaxViewports()
{
    GLint value = 0;
    glGetIntegerv(GL_MAX_VIEWPORTS, &value);
    return value;
}

void GetTexParameterInternalFormat(GLenum target, GLint* params)
{
    glGetTexLevelParameteriv(target, 0, GL_TEXTURE_INTERNAL_FORMAT, params);
}

void GetInternalformativ(GLenum target, GLenum internalformat, GLenum pname, GLsizei bufsize, GLint* params)
{
    #ifdef GL_ARB_internalformat_query
    glGetInternalformativ(target, internalformat, pname, bufsize, params);
    #endif
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
    glFramebufferTextureLayer(target, attachment, texture, level, layer);
}


} // /namespace GLProfile

} // /namespace LLGL



// ================================================================================
