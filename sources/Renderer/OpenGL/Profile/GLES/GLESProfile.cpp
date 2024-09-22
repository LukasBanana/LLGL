/*
 * GLESProfile.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "../GLProfile.h"
#include "../../Ext/GLExtensions.h"
#include "../../../../Core/Assertion.h"
#include <LLGL/RenderSystemFlags.h>
#include <cstring>


namespace LLGL
{

namespace GLProfile
{


int GetRendererID()
{
    return RendererID::OpenGLES;
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

OpenGLContextProfile GetContextProfile()
{
    return OpenGLContextProfile::ESProfile;
}

GLint GetMaxViewports()
{
    return 1;
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
    LLGL_ASSERT_PTR(data);
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

void* MapBufferRange(GLenum target, GLintptr offset, GLsizeiptr length, GLbitfield access)
{
    return glMapBufferRange(target, offset, length, access);
}

void UnmapBuffer(GLenum target)
{
    glUnmapBuffer(target);
}

// Global memory bank of CL_COLOR_ATTACHMENT0-7 values initialized with GL_NONE
static GLenum g_colorAttachmentsBank[8] =
{
    GL_NONE, GL_NONE, GL_NONE, GL_NONE,
    GL_NONE, GL_NONE, GL_NONE, GL_NONE,
    #if LLGL_MAX_NUM_COLOR_ATTACHMENTS > 8
    GL_NONE, GL_NONE, GL_NONE, GL_NONE,
    GL_NONE, GL_NONE, GL_NONE, GL_NONE,
    #endif
};

static_assert(LLGL_MAX_NUM_COLOR_ATTACHMENTS <= 16, "GLES profile can only handle up to 16 color attachments, but LLGL_MAX_NUM_COLOR_ATTACHMENTS exceed that limit");

void DrawBuffer(GLenum buf)
{
    if (buf >= GL_COLOR_ATTACHMENT0 && buf <= GL_COLOR_ATTACHMENT0 + LLGL_MAX_NUM_COLOR_ATTACHMENTS)
    {
        /*
        GL_COLOR_ATTACHMENT(i) must only be used at the i-th binding point in GLES/WebGL,
        so maintain a bank of GL_NONE entries and swap out the corresponding slot with the input attachment.
        */
        const GLsizei attachmentIndex = (buf - GL_COLOR_ATTACHMENT0);
        g_colorAttachmentsBank[attachmentIndex] = buf;
        glDrawBuffers(attachmentIndex + 1, g_colorAttachmentsBank);
        g_colorAttachmentsBank[attachmentIndex] = GL_NONE;
    }
    else
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
