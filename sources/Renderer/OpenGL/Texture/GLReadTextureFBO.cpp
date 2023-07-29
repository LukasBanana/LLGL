/*
 * GLReadTextureFBO.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "GLReadTextureFBO.h"
#include "GLTexture.h"
#include "../RenderState/GLStateManager.h"
#include "../GLTypes.h"
#include <LLGL/TextureFlags.h>


namespace LLGL
{


GLReadTextureFBO::GLReadTextureFBO()
{
    fbo_.GenFramebuffer();
    GLStateManager::Get().BindFramebuffer(GLFramebufferTarget::ReadFramebuffer, fbo_.GetID());
}

GLReadTextureFBO::~GLReadTextureFBO()
{
    fbo_.DeleteFramebuffer();
}

// Converts the corresponding offset component into the array layer with respect to the texture type
static GLint TextureOffsetToArrayLayer(const TextureType type, const Offset3D& offset)
{
    switch (type)
    {
        case TextureType::Texture1DArray:
            return static_cast<GLint>(offset.y);
        case TextureType::Texture3D:
        case TextureType::Texture2DArray:
        case TextureType::Texture2DMSArray:
        case TextureType::TextureCube:
        case TextureType::TextureCubeArray:
            return offset.z;
        default:
            return 0;
    }
}

static GLenum GetGLAttachmentForInternalFormat(GLenum internalFormat)
{
    if (GLTypes::IsDepthFormat(internalFormat))
        return GL_DEPTH_ATTACHMENT;
    if (GLTypes::IsDepthStencilFormat(internalFormat))
        return GL_DEPTH_STENCIL_ATTACHMENT;
    return GL_COLOR_ATTACHMENT0;
}

void GLReadTextureFBO::Attach(GLTexture& texture, GLint mipLevel, const Offset3D& offset)
{
    const GLenum attachment = GetGLAttachmentForInternalFormat(texture.GetGLInternalFormat());
    GLFramebuffer::AttachTexture(
        texture,
        attachment,
        static_cast<GLint>(mipLevel),
        TextureOffsetToArrayLayer(texture.GetType(), offset),
        GL_READ_FRAMEBUFFER
    );
}


} // /namespace LLGL



// ================================================================================
