/*
 * GLTexture.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLTexture.h"
#include "../RenderState/GLStateManager.h"
#include "../GLTypes.h"


namespace LLGL
{


GLTexture::GLTexture()
{
    glGenTextures(1, &id_);
}

GLTexture::~GLTexture()
{
    glDeleteTextures(1, &id_);
}

Gs::Vector3i GLTexture::QueryMipLevelSize(int mipLevel) const
{
    Gs::Vector3i size;

    GLStateManager::active->PushBoundTexture(GLStateManager::GetTextureTarget(GetType()));
    {
        GLStateManager::active->BindTexture(*this);

        auto target = GLTypes::Map(GetType());
        glGetTexLevelParameteriv(target, mipLevel, GL_TEXTURE_WIDTH, &size.x);
        glGetTexLevelParameteriv(target, mipLevel, GL_TEXTURE_HEIGHT, &size.y);
        glGetTexLevelParameteriv(target, mipLevel, GL_TEXTURE_DEPTH, &size.z);
    }
    GLStateManager::active->PopBoundTexture();

    return size;
}

void GLTexture::Recreate()
{
    /* Delete previous texture and create a new one */
    glDeleteTextures(1, &id_);
    glGenTextures(1, &id_);
}


/*
 * ======= Protected: =======
 */

void GLTexture::SetType(const TextureType type)
{
    Texture::SetType(type);
}


} // /namespace LLGL



// ================================================================================
