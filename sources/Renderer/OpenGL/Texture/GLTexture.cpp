/*
 * GLTexture.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLTexture.h"
#include "../RenderState/GLStateManager.h"
#include "../../GLCommon/GLTypes.h"
#include "../../GLCommon/GLExtensionRegistry.h"
#include "../Ext/GLExtensions.h"


namespace LLGL
{


GLTexture::GLTexture(const TextureType type) :
    Texture { type }
{
    AllocHwTexture();
}

GLTexture::~GLTexture()
{
    FreeHwTexture();
}

Gs::Vector3ui GLTexture::QueryMipLevelSize(std::uint32_t mipLevel) const
{
    auto target = GLTypes::Map(GetType());

    GLint texSize[3] = { 0 };
    GLint level = static_cast<GLint>(mipLevel);

    #if defined GL_ARB_direct_state_access && defined LLGL_GL_ENABLE_DSA_EXT
    if (HasExtension(GLExt::ARB_direct_state_access))
    {
        /* Query texture attributes directly using DSA */
        glGetTextureLevelParameteriv(id_, level, GL_TEXTURE_WIDTH,  &texSize[0]);
        glGetTextureLevelParameteriv(id_, level, GL_TEXTURE_HEIGHT, &texSize[1]);
        glGetTextureLevelParameteriv(id_, level, GL_TEXTURE_DEPTH,  &texSize[2]);
    }
    else
    #endif
    {
        /* Push currently bound texture onto stack to restore it after query */
        GLStateManager::active->PushBoundTexture(GLStateManager::GetTextureTarget(GetType()));
        {
            /* Bind texture and query attributes */
            GLStateManager::active->BindTexture(*this);
            glGetTexLevelParameteriv(target, level, GL_TEXTURE_WIDTH,  &texSize[0]);
            glGetTexLevelParameteriv(target, level, GL_TEXTURE_HEIGHT, &texSize[1]);
            glGetTexLevelParameteriv(target, level, GL_TEXTURE_DEPTH,  &texSize[2]);
        }
        GLStateManager::active->PopBoundTexture();
    }

    return Gs::Vector3ui
    {
        static_cast<std::uint32_t>(texSize[0]),
        static_cast<std::uint32_t>(texSize[1]),
        static_cast<std::uint32_t>(texSize[2])
    };
}

TextureDescriptor GLTexture::QueryDesc() const
{
    TextureDescriptor desc;

    desc.type = GetType();

    /* Query hardware texture format and size */
    GLint internalFormat = 0, texSize[3] = { 0 };

    #if defined GL_ARB_direct_state_access && defined LLGL_GL_ENABLE_DSA_EXT
    if (HasExtension(GLExt::ARB_direct_state_access))
    {
        /* Query texture attributes directly using DSA */
        glGetTextureLevelParameteriv(id_, 0, GL_TEXTURE_INTERNAL_FORMAT, &internalFormat);
        glGetTextureLevelParameteriv(id_, 0, GL_TEXTURE_WIDTH, &texSize[0]);
        glGetTextureLevelParameteriv(id_, 0, GL_TEXTURE_HEIGHT, &texSize[1]);
        glGetTextureLevelParameteriv(id_, 0, GL_TEXTURE_DEPTH, &texSize[2]);
    }
    else
    #endif
    {
        /* Push currently bound texture onto stack to restore it after query */
        GLStateManager::active->PushBoundTexture(GLStateManager::GetTextureTarget(GetType()));
        {
            /* Bind texture and query attributes */
            GLStateManager::active->BindTexture(*this);
            auto target = GLTypes::Map(GetType());
            glGetTexLevelParameteriv(target, 0, GL_TEXTURE_INTERNAL_FORMAT, &internalFormat);
            glGetTexLevelParameteriv(target, 0, GL_TEXTURE_WIDTH, &texSize[0]);
            glGetTexLevelParameteriv(target, 0, GL_TEXTURE_HEIGHT, &texSize[1]);
            glGetTexLevelParameteriv(target, 0, GL_TEXTURE_DEPTH, &texSize[2]);
        }
        GLStateManager::active->PopBoundTexture();
    }

    /* Transform data from OpenGL to LLGL */
    GLTypes::Unmap(desc.format, static_cast<GLenum>(internalFormat));

    desc.texture3D.width    = static_cast<std::uint32_t>(texSize[0]);
    desc.texture3D.height   = static_cast<std::uint32_t>(texSize[1]);
    desc.texture3D.depth    = static_cast<std::uint32_t>(texSize[2]);

    if (desc.type == TextureType::TextureCube || desc.type == TextureType::TextureCubeArray)
        desc.texture3D.depth /= 6;

    return desc;
}

void GLTexture::Recreate()
{
    /* Delete previous texture and create a new one */
    FreeHwTexture();
    AllocHwTexture();
}


/*
 * ======= Private: =======
 */

void GLTexture::AllocHwTexture()
{
    #if defined GL_ARB_direct_state_access && defined LLGL_GL_ENABLE_DSA_EXT
    if (HasExtension(GLExt::ARB_direct_state_access))
    {
        /* Create new GL texture object with respective target */
        glCreateTextures(GLTypes::Map(GetType()), 1, &id_);
    }
    else
    #endif
    {
        /* Create new GL texture object (must be bound to a target before it can be used) */
        glGenTextures(1, &id_);
    }
}

void GLTexture::FreeHwTexture()
{
    glDeleteTextures(1, &id_);
}


} // /namespace LLGL



// ================================================================================
