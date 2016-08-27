/*
 * GLFrameBuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLFrameBuffer.h"
#include "GLRenderBuffer.h"
#include "../GLExtensions.h"
#include "../RenderState/GLStateManager.h"


namespace LLGL
{


static GLFrameBufferTarget GetFrameBufferTarget(GLenum target)
{
    return (target == GL_READ_FRAMEBUFFER ? GLFrameBufferTarget::READ_FRAMEBUFFER : GLFrameBufferTarget::DRAW_FRAMEBUFFER);
}

GLFrameBuffer::GLFrameBuffer(GLenum target) :
    target_( target )
{
    glGenFramebuffers(1, &id_);
}

GLFrameBuffer::~GLFrameBuffer()
{
    glDeleteFramebuffers(1, &id_);
}

void GLFrameBuffer::Bind() const
{
    GLStateManager::active->BindFrameBuffer(GetFrameBufferTarget(target_), id_);
}

void GLFrameBuffer::Unbind() const
{
    GLStateManager::active->BindFrameBuffer(GetFrameBufferTarget(target_), 0);
}

void GLFrameBuffer::Recreate()
{
    /* Delete previous framebuffer and create a new one */
    glDeleteFramebuffers(1, &id_);
    glGenFramebuffers(1, &id_);
}

void GLFrameBuffer::AttachTexture1D(GLenum attachment, GLTexture& texture, GLenum textureTarget, GLint mipLevel)
{
    glFramebufferTexture1D(target_, attachment, textureTarget, texture.GetID(), mipLevel);
}

void GLFrameBuffer::AttachTexture2D(GLenum attachment, GLTexture& texture, GLenum textureTarget, GLint mipLevel)
{
    glFramebufferTexture2D(target_, attachment, textureTarget, texture.GetID(), mipLevel);
}

void GLFrameBuffer::AttachTexture3D(GLenum attachment, GLTexture& texture, GLenum textureTarget, GLint mipLevel, GLint zOffset)
{
    glFramebufferTexture3D(target_, attachment, textureTarget, texture.GetID(), mipLevel, zOffset);
}

void GLFrameBuffer::AttachTextureLayer(GLenum attachment, GLTexture& texture, GLint mipLevel, GLint layer)
{
    glFramebufferTextureLayer(target_, attachment, texture.GetID(), mipLevel, layer);
}

void GLFrameBuffer::AttachRenderBuffer(GLenum attachment, GLRenderBuffer& renderBuffer)
{
    glFramebufferRenderbuffer(target_, attachment, GL_RENDERBUFFER, renderBuffer.GetID());
}

void GLFrameBuffer::Blit(const Gs::Vector2i& size, GLenum mask, GLenum filter)
{
    glBlitFramebuffer(
        0, 0, size.x, size.y,
        0, 0, size.x, size.y,
        mask, filter
    );
}

void GLFrameBuffer::Blit(
    const Gs::Vector2i& srcPos0, const Gs::Vector2i& srcPos1,
    const Gs::Vector2i& destPos0, const Gs::Vector2i& destPos1,
    GLenum mask, GLenum filter)
{
    glBlitFramebuffer(
        srcPos0.x, srcPos0.y, srcPos1.x, srcPos1.y,
        destPos0.x, destPos0.y, destPos1.x, destPos1.y,
        mask, filter
    );
}

GLenum GLFrameBuffer::CheckStatus() const
{
    return glCheckFramebufferStatus(target_);
}


} // /namespace LLGL



// ================================================================================
