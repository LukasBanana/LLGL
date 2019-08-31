/*
 * GLReadTextureFBO.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLReadTextureFBO.h"
#include "GLTexture.h"
#include "../RenderState/GLStateManager.h"
#include <LLGL/TextureFlags.h>


namespace LLGL
{


GLReadTextureFBO::GLReadTextureFBO()
{
    fbo_.GenFramebuffer();
    fbo_.Bind(GLFramebufferTarget::READ_FRAMEBUFFER);
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

void GLReadTextureFBO::Attach(GLTexture& texture, GLint mipLevel, const Offset3D& offset)
{
    GLFramebuffer::AttachTexture(
        texture,
        GL_COLOR_ATTACHMENT0,
        static_cast<GLint>(mipLevel),
        TextureOffsetToArrayLayer(texture.GetType(), offset),
        GL_READ_FRAMEBUFFER
    );
}


} // /namespace LLGL



// ================================================================================
