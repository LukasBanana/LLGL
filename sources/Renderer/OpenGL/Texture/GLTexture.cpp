/*
 * GLTexture.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLTexture.h"
#include "../RenderState/GLStateManager.h"
#include "../../GLCommon/GLTypes.h"


namespace LLGL
{


GLTexture::GLTexture(const TextureType type) :
    Texture( type )
{
    glGenTextures(1, &id_);
}

GLTexture::~GLTexture()
{
    glDeleteTextures(1, &id_);
}

Gs::Vector3ui GLTexture::QueryMipLevelSize(unsigned int mipLevel) const
{
    Gs::Vector3ui size;

    GLStateManager::active->PushBoundTexture(GLStateManager::GetTextureTarget(GetType()));
    {
        GLStateManager::active->BindTexture(*this);

        auto target = GLTypes::Map(GetType());

        GLint texSize[3] = { 0 };
        glGetTexLevelParameteriv(target, mipLevel, GL_TEXTURE_WIDTH,  &texSize[0]);
        glGetTexLevelParameteriv(target, mipLevel, GL_TEXTURE_HEIGHT, &texSize[1]);
        glGetTexLevelParameteriv(target, mipLevel, GL_TEXTURE_DEPTH,  &texSize[2]);

        size.x = static_cast<unsigned int>(texSize[0]);
        size.y = static_cast<unsigned int>(texSize[1]);
        size.z = static_cast<unsigned int>(texSize[2]);
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


} // /namespace LLGL



// ================================================================================
