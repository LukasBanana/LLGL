/*
 * GLESProfile.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "../GLProfile.h"
#include <LLGL/RenderSystemFlags.h>
#include <cstring>


namespace LLGL
{

namespace GLProfile
{


int GetRendererID()
{
    return RendererID::OpenGLES3;
}

const char* GetModuleName()
{
    return "OpenGLES3";
}

const char* GetRendererName()
{
    return "OpenGL ES 3";
}

const char* GetAPIName()
{
    return "OpenGL ES";
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
    #ifdef GL_ES_VERSION_3_1
    glGetTexLevelParameteriv(target, 0, GL_TEXTURE_INTERNAL_FORMAT, params);
    #else
    //TODO...
    #endif
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
    if (void* srcData = glMapBufferRange(target, offset, size, GL_MAP_READ_BIT))
    {
        ::memcpy(data, srcData, static_cast<std::size_t>(size));
        glUnmapBuffer(target);
    }
}

static GLbitfield ToGLESMapBufferRangeAccess(GLenum access)
{
    switch (access)
    {
        case GL_READ_ONLY:  return GL_MAP_READ_BIT;
        case GL_WRITE_ONLY: return GL_MAP_WRITE_BIT;
        case GL_READ_WRITE: return GL_MAP_READ_BIT | GL_MAP_WRITE_BIT;
        default:            return 0;
    }
}

void* MapBuffer(GLenum target, GLenum access)
{
    /* Translate GL access type to GLES bitfield, determine buffer length, and map entire buffer range */
    GLbitfield flags = ToGLESMapBufferRangeAccess(access);
    GLint length = 0;
    glGetBufferParameteriv(target, GL_BUFFER_SIZE, &length);
    return glMapBufferRange(target, 0, length, flags);
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
