/*
 * GLFramebuffer.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "GLFramebuffer.h"
#include "GLTexture.h"
#include "../Profile/GLProfile.h"
#include "../GLTypes.h"
#include "../Ext/GLExtensions.h"
#include "../Ext/GLExtensionRegistry.h"
#include "../RenderState/GLStateManager.h"


namespace LLGL
{


/*
 * GLFramebufferPair structure
 */

GLFramebufferPair::~GLFramebufferPair()
{
    ReleaseFBOs();
}

void GLFramebufferPair::CreateFBOs()
{
    if (fbos[0] == 0)
        glGenFramebuffers(2, fbos);
}

void GLFramebufferPair::ReleaseFBOs()
{
    if (fbos[0] != 0)
    {
        glDeleteFramebuffers(2, fbos);
        fbos[0] = 0;
        fbos[1] = 0;
    }
}


/*
 * GLFramebuffer class
 */

GLFramebuffer::~GLFramebuffer()
{
    DeleteFramebuffer();
}

GLFramebuffer::GLFramebuffer(GLFramebuffer&& rhs) :
    id_ { rhs.id_ }
{
    rhs.id_ = 0;
}

GLFramebuffer& GLFramebuffer::operator = (GLFramebuffer&& rhs)
{
    if (id_ != rhs.id_)
    {
        DeleteFramebuffer();
        id_ = rhs.id_;
        rhs.id_ = 0;
    }
    return *this;
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
        GLStateManager::Get().NotifyFramebufferRelease(id_);
        id_ = 0;
    }
}

bool GLFramebuffer::FramebufferParameters(
    GLint width,
    GLint height,
    GLint layers,
    GLint samples,
    GLint fixedSampleLocations)
{
    #if LLGL_GLEXT_FRAMEBUFFER_NO_ATTACHMENTS
    if (HasExtension(GLExt::ARB_framebuffer_no_attachments))
    {
        GLStateManager::Get().BindFramebuffer(GLFramebufferTarget::Framebuffer, GetID());
        glFramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_WIDTH, width);
        glFramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_HEIGHT, height);
        glFramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_LAYERS, layers);
        glFramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_SAMPLES, samples);
        glFramebufferParameteri(GL_FRAMEBUFFER, GL_FRAMEBUFFER_DEFAULT_FIXED_SAMPLE_LOCATIONS, fixedSampleLocations);
        return true;
    }
    #endif // /LLGL_GLEXT_FRAMEBUFFER_NO_ATTACHMENTS
    return false;
}

void GLFramebuffer::AttachTexture(
    const GLTexture&    texture,
    GLenum              attachment,
    GLint               mipLevel,
    GLint               arrayLayer,
    GLenum              target)
{
    GLuint texID = texture.GetID();
    #if LLGL_GLEXT_FRAMEBUFFER_OBJECT
    if (texture.IsRenderbuffer())
    {
        /* Attach renderbuffer to FBO */
        glFramebufferRenderbuffer(target, attachment, GL_RENDERBUFFER, texID);
    }
    else
    #endif // /LLGL_GLEXT_FRAMEBUFFER_OBJECT
    {
        /* Attach texture to FBO */
        switch (texture.GetType())
        {
            case TextureType::Texture1D:
                GLProfile::FramebufferTexture1D(target, attachment, GL_TEXTURE_1D, texID, mipLevel);
                break;
            case TextureType::Texture2D:
                GLProfile::FramebufferTexture2D(target, attachment, GL_TEXTURE_2D, texID, mipLevel);
                break;
            case TextureType::Texture3D:
                GLProfile::FramebufferTexture3D(target, attachment, GL_TEXTURE_3D, texID, mipLevel, arrayLayer);
                break;
            case TextureType::TextureCube:
                GLProfile::FramebufferTexture2D(target, attachment, GLTypes::ToTextureCubeMap(static_cast<std::uint32_t>(arrayLayer)), texID, mipLevel);
                break;
            #if !LLGL_GL_ENABLE_OPENGL2X
            case TextureType::Texture1DArray:
            case TextureType::Texture2DArray:
            case TextureType::TextureCubeArray:
                GLProfile::FramebufferTextureLayer(target, attachment, texID, mipLevel, arrayLayer);
                break;
            case TextureType::Texture2DMS:
                GLProfile::FramebufferTexture2D(target, attachment, GL_TEXTURE_2D_MULTISAMPLE, texID, 0);
                break;
            case TextureType::Texture2DMSArray:
                GLProfile::FramebufferTextureLayer(target, attachment, texID, 0, arrayLayer);
                break;
            #else // !LLGL_GL_ENABLE_OPENGL2X
            default:
                LLGL_TRAP_FEATURE_NOT_SUPPORTED("array- & multi-sampled textures");
                break;
            #endif // /!LLGL_GL_ENABLE_OPENGL2X
        }
    }
}

void GLFramebuffer::AttachRenderbuffer(GLenum attachment, GLuint renderbufferID)
{
    #if LLGL_GLEXT_FRAMEBUFFER_OBJECT
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, renderbufferID);
    #else
    LLGL_TRAP_FEATURE_NOT_SUPPORTED("renderbuffer");
    #endif
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


} // /namespace LLGL



// ================================================================================
