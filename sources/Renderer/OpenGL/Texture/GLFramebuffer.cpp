/*
 * GLFramebuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
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

//TODO: remove this as soon as "RenderTarget::Detach" is removed!
#if 1
void GLFramebuffer::Recreate()
{
    /* Delete previous framebuffer and create a new one */
    glDeleteFramebuffers(1, &id_);
    glGenFramebuffers(1, &id_);
}
#endif

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

void GLFramebuffer::Blit(GLint width, GLint height, GLenum mask)
{
    glBlitFramebuffer(
        0, 0, width, height,
        0, 0, width, height,
        mask, GL_NEAREST
    );
}

void GLFramebuffer::Blit(
    const Offset2D& srcPos0, const Offset2D& srcPos1,
    const Offset2D& destPos0, const Offset2D& destPos1,
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
