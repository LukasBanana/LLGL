/*
 * GLFramebuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLFramebuffer.h"
#include "../Ext/GLExtensions.h"
#include "../RenderState/GLStateManager.h"


namespace LLGL
{


GLFramebuffer::GLFramebuffer()
{
    glGenFramebuffers(1, &id_);
}

GLFramebuffer::~GLFramebuffer()
{
    glDeleteFramebuffers(1, &id_);
}

void GLFramebuffer::Bind(const GLFramebufferTarget target) const
{
    GLStateManager::active->BindFramebuffer(target, id_);
}

void GLFramebuffer::Unbind(const GLFramebufferTarget target) const
{
    GLStateManager::active->BindFramebuffer(target, 0);
}

void GLFramebuffer::Recreate()
{
    /* Delete previous framebuffer and create a new one */
    glDeleteFramebuffers(1, &id_);
    glGenFramebuffers(1, &id_);
}

void GLFramebuffer::AttachTexture1D(GLenum attachment, GLenum textureTarget, GLuint textureID, GLint mipLevel)
{
    glFramebufferTexture1D(GL_FRAMEBUFFER, attachment, textureTarget, textureID, mipLevel);
}

void GLFramebuffer::AttachTexture2D(GLenum attachment, GLenum textureTarget, GLuint textureID, GLint mipLevel)
{
    glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, textureTarget, textureID, mipLevel);
}

void GLFramebuffer::AttachTexture3D(GLenum attachment, GLenum textureTarget, GLuint textureID, GLint mipLevel, GLint zOffset)
{
    glFramebufferTexture3D(GL_FRAMEBUFFER, attachment, textureTarget, textureID, mipLevel, zOffset);
}

void GLFramebuffer::AttachTextureLayer(GLenum attachment, GLuint textureID, GLint mipLevel, GLint layer)
{
    glFramebufferTextureLayer(GL_FRAMEBUFFER, attachment, textureID, mipLevel, layer);
}

void GLFramebuffer::AttachRenderbuffer(GLenum attachment, GLuint renderbufferID)
{
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, renderbufferID);
}

void GLFramebuffer::Blit(const Gs::Vector2i& size, GLenum mask)
{
    glBlitFramebuffer(
        0, 0, size.x, size.y,
        0, 0, size.x, size.y,
        mask, GL_NEAREST
    );
}

void GLFramebuffer::Blit(
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


} // /namespace LLGL



// ================================================================================
