/*
 * GLFramebuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLFramebuffer.h"
#include "../../GLCommon/GLExtensionRegistry.h"
#include "../Ext/GLExtensions.h"
#include "../RenderState/GLStateManager.h"


namespace LLGL
{


GLFramebuffer::~GLFramebuffer()
{
    DeleteFramebuffer();
}

void GLFramebuffer::GenFramebuffer()
{
    DeleteFramebuffer();
    glGenFramebuffers(1, &id_);
}

void GLFramebuffer::DeleteFramebuffer()
{
    if (id_ != 0)
    {
        glDeleteFramebuffers(1, &id_);
        GLStateManager::active->NotifyFramebufferRelease(id_);
        id_ = 0;
    }
}

void GLFramebuffer::Bind(const GLFramebufferTarget target) const
{
    GLStateManager::active->BindFramebuffer(target, GetID());
}

void GLFramebuffer::Unbind(const GLFramebufferTarget target) const
{
    GLStateManager::active->BindFramebuffer(target, 0);
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

void GLFramebuffer::Blit(GLint width, GLint height, GLenum mask)
{
    glBlitFramebuffer(
        0, 0, width, height,
        0, 0, width, height,
        mask, GL_NEAREST
    );
}

void GLFramebuffer::Blit(
    const Offset2D& srcPos0,
    const Offset2D& srcPos1,
    const Offset2D& destPos0,
    const Offset2D& destPos1,
    GLenum          mask,
    GLenum          filter)
{
    glBlitFramebuffer(
        srcPos0.x, srcPos0.y, srcPos1.x, srcPos1.y,
        destPos0.x, destPos0.y, destPos1.x, destPos1.y,
        mask, filter
    );
}

bool GLFramebuffer::FramebufferParameters(
    GLint width,
    GLint height,
    GLint layers,
    GLint samples,
    GLint fixedSampleLocations)
{
    #ifdef GL_ARB_framebuffer_no_attachments
    if (HasExtension(GLExt::ARB_framebuffer_no_attachments))
    {
        GLStateManager::active->BindFramebuffer(GLFramebufferTarget::FRAMEBUFFER, GetID());
        glFramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_WIDTH, width);
        glFramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_HEIGHT, height);
        glFramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_LAYERS, layers);
        glFramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_SAMPLES, samples);
        glFramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_FIXED_SAMPLE_LOCATIONS, fixedSampleLocations);
        return true;
    }
    #endif // /GL_ARB_framebuffer_no_attachments
    return false;
}


} // /namespace LLGL



// ================================================================================
