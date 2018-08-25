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

GLTexture::~GLTexture()
{
    glDeleteTextures(1, &id_);
    GLStateManager::active->NotifyTextureRelease(id_, GLStateManager::GetTextureTarget(GetType()));
}

static GLenum GLGetTextureParamTarget(const TextureType type)
{
    switch (type)
    {
        case TextureType::Texture1D:        return GL_TEXTURE_1D;
        case TextureType::Texture2D:        return GL_TEXTURE_2D;
        case TextureType::Texture3D:        return GL_TEXTURE_3D;
        case TextureType::TextureCube:      return GL_TEXTURE_CUBE_MAP_POSITIVE_X; // use first cube face instead of texture type
        case TextureType::Texture1DArray:   return GL_TEXTURE_1D_ARRAY;
        case TextureType::Texture2DArray:   return GL_TEXTURE_2D_ARRAY;
        case TextureType::TextureCubeArray: return GL_TEXTURE_CUBE_MAP_POSITIVE_X; // use first cube face instead of texture type
        case TextureType::Texture2DMS:      return GL_TEXTURE_2D_MULTISAMPLE;
        case TextureType::Texture2DMSArray: return GL_TEXTURE_2D_MULTISAMPLE_ARRAY;
    }
    return 0;
}

Extent3D GLTexture::QueryMipExtent(std::uint32_t mipLevel) const
{
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
            auto target = GLGetTextureParamTarget(GetType());
            glGetTexLevelParameteriv(target, level, GL_TEXTURE_WIDTH,  &texSize[0]);
            glGetTexLevelParameteriv(target, level, GL_TEXTURE_HEIGHT, &texSize[1]);
            glGetTexLevelParameteriv(target, level, GL_TEXTURE_DEPTH,  &texSize[2]);
        }
        GLStateManager::active->PopBoundTexture();
    }

    /* Adjust depth value for cube texture to be uniform with D3D */
    if (IsCubeTexture(GetType()))
        texSize[2] *= 6;

    return
    {
        static_cast<std::uint32_t>(texSize[0]),
        static_cast<std::uint32_t>(texSize[1]),
        static_cast<std::uint32_t>(texSize[2])
    };
}

TextureDescriptor GLTexture::QueryDesc() const
{
    TextureDescriptor texDesc;

    texDesc.type    = GetType();
    texDesc.flags   = 0;

    /* Query hardware texture format and size */
    GLint internalFormat = 0, extent[3] = { 0 };
    QueryTexParams(&internalFormat, extent);

    /*
    Transform data from OpenGL to LLGL
    NOTE: for cube textures, depth extent can also be copied directly without transformation (no need to multiply by 6)
    */
    texDesc.format              = GLTypes::UnmapFormat(static_cast<GLenum>(internalFormat));

    texDesc.extent.width        = static_cast<std::uint32_t>(extent[0]);
    texDesc.extent.height       = static_cast<std::uint32_t>(extent[1]);

    if (GetType() == TextureType::Texture3D)
        texDesc.extent.depth    = static_cast<std::uint32_t>(extent[2]);
    else
        texDesc.arrayLayers     = static_cast<std::uint32_t>(extent[2]);

    return texDesc;
}

GLenum GLTexture::QueryGLInternalFormat() const
{
    /* Query hardware texture format */
    GLint internalFormat = 0;
    QueryTexParams(&internalFormat, nullptr);
    return static_cast<GLenum>(internalFormat);
}


/*
 * ======= Private: =======
 */

void GLTexture::QueryTexParams(GLint* internalFormat, GLint* extent) const
{
    #if defined GL_ARB_direct_state_access && defined LLGL_GL_ENABLE_DSA_EXT
    if (HasExtension(GLExt::ARB_direct_state_access))
    {
        /* Query texture attributes directly using DSA */
        if (internalFormat)
            glGetTextureLevelParameteriv(id_, 0, GL_TEXTURE_INTERNAL_FORMAT, internalFormat);

        if (extent)
        {
            glGetTextureLevelParameteriv(id_, 0, GL_TEXTURE_WIDTH,  &extent[0]);
            glGetTextureLevelParameteriv(id_, 0, GL_TEXTURE_HEIGHT, &extent[1]);
            glGetTextureLevelParameteriv(id_, 0, GL_TEXTURE_DEPTH,  &extent[2]);
        }
    }
    else
    #endif
    {
        /* Push currently bound texture onto stack to restore it after query */
        GLStateManager::active->PushBoundTexture(GLStateManager::GetTextureTarget(GetType()));
        {
            /* Bind texture and query attributes */
            GLStateManager::active->BindTexture(*this);
            auto target = GLGetTextureParamTarget(GetType());

            if (internalFormat)
                glGetTexLevelParameteriv(target, 0, GL_TEXTURE_INTERNAL_FORMAT, internalFormat);

            if (extent)
            {
                glGetTexLevelParameteriv(target, 0, GL_TEXTURE_WIDTH,  &extent[0]);
                glGetTexLevelParameteriv(target, 0, GL_TEXTURE_HEIGHT, &extent[1]);
                glGetTexLevelParameteriv(target, 0, GL_TEXTURE_DEPTH,  &extent[2]);
            }
        }
        GLStateManager::active->PopBoundTexture();
    }
}


} // /namespace LLGL



// ================================================================================
