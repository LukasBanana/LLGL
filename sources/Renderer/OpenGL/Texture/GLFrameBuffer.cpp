/*
 * GLFrameBuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLFrameBuffer.h"
#include "../GLExtensions.h"
#include "../RenderState/GLStateManager.h"


namespace LLGL
{


GLFrameBuffer::GLFrameBuffer()
{
    glGenFramebuffers(1, &id_);
}

GLFrameBuffer::~GLFrameBuffer()
{
    glDeleteFramebuffers(1, &id_);
}

void GLFrameBuffer::Bind()
{
    GLStateManager::active->BindFrameBuffer(GLFrameBufferTarget::DRAW_FRAMEBUFFER, id_);
}

void GLFrameBuffer::Unbind()
{
    GLStateManager::active->BindFrameBuffer(GLFrameBufferTarget::DRAW_FRAMEBUFFER, 0);
}

void GLFrameBuffer::Recreate()
{
    /* Delete previous framebuffer and create a new one */
    glDeleteFramebuffers(1, &id_);
    glGenFramebuffers(1, &id_);
}

void GLFrameBuffer::AttachTexture1D(GLenum attachment, GLTexture& texture, GLenum textureTarget, GLint mipLevel)
{
    glFramebufferTexture1D(GL_DRAW_FRAMEBUFFER, attachment, textureTarget, texture.GetID(), mipLevel);
}

void GLFrameBuffer::AttachTexture2D(GLenum attachment, GLTexture& texture, GLenum textureTarget, GLint mipLevel)
{
    glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, attachment, textureTarget, texture.GetID(), mipLevel);
}

void GLFrameBuffer::AttachTexture3D(GLenum attachment, GLTexture& texture, GLenum textureTarget, GLint mipLevel, GLint zOffset)
{
    glFramebufferTexture3D(GL_DRAW_FRAMEBUFFER, attachment, textureTarget, texture.GetID(), mipLevel, zOffset);
}

void GLFrameBuffer::AttachTextureLayer(GLenum attachment, GLTexture& texture, GLint mipLevel, GLint layer)
{
    glFramebufferTextureLayer(GL_DRAW_FRAMEBUFFER, attachment, texture.GetID(), mipLevel, layer);
}


} // /namespace LLGL



// ================================================================================
