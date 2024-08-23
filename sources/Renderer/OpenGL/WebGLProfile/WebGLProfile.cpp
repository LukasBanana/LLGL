/*
 * WebGLProfile.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "../GLProfile.h"
#include "../Ext/GLExtensions.h"
#include <LLGL/RenderSystemFlags.h>
#include <cstring>


namespace LLGL
{

namespace GLProfile
{


int GetRendererID()
{
    return RendererID::WebGL;
}

const char* GetModuleName()
{
    return "WebGL";
}

const char* GetRendererName()
{
    return "WebGL";
}

const char* GetAPIName()
{
    return "WebGL";
}

const char* GetShadingLanguageName()
{
    return "ESSL";
}

GLint GetMaxViewports()
{
    return 1;
}

void GetTexParameterInternalFormat(GLenum target, GLint* params)
{
    //TODO...
}

void GetInternalformativ(GLenum target, GLenum internalformat, GLenum pname, GLsizei bufsize, GLint* params)
{
    //TODO
}

void DepthRange(GLclamp_t nearVal, GLclamp_t farVal)
{
    glDepthRangef(nearVal, farVal);
}

void ClearDepth(GLclamp_t depth)
{
    glClearDepthf(depth);
}

void GetBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, void* data)
{
    // dummy
}

void* MapBuffer(GLenum target, GLenum access)
{
    return nullptr; // dummy
}

void* MapBufferRange(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access)
{
    return nullptr; // dummy
}

void UnmapBuffer(GLenum target)
{
    // dummy
}

void DrawBuffer(GLenum buf)
{
    glDrawBuffers(1, &buf);
}

void FramebufferTexture1D(GLenum /*target*/, GLenum /*attachment*/, GLenum /*textarget*/, GLuint /*texture*/, GLint /*level*/)
{
    // dummy
}

void FramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
    glFramebufferTexture2D(target, attachment, textarget, texture, level);
}

void FramebufferTexture3D(GLenum target, GLenum attachment, GLenum /*textarget*/, GLuint texture, GLint level, GLint layer)
{
    glFramebufferTextureLayer(target, attachment, texture, level, layer);
}

void FramebufferTextureLayer(GLenum target, GLenum attachment, GLuint texture, GLint level, GLint layer)
{
    glFramebufferTextureLayer(target, attachment, texture, level, layer);
}


} // /namespace GLProfile

} // /namespace LLGL



// ================================================================================
