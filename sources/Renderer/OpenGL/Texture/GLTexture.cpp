/*
 * GLTexture.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLTexture.h"
#include "GLReadTextureFBO.h"
#include "../GLObjectUtils.h"
#include "../RenderState/GLStateManager.h"
#include "../../GLCommon/GLTypes.h"
#include "../../GLCommon/GLExtensionRegistry.h"
#include "../Ext/GLExtensions.h"


namespace LLGL
{


GLTexture::GLTexture(const TextureDescriptor& desc) :
    Texture       { desc.type                                },
    numMipLevels_ { static_cast<GLsizei>(NumMipLevels(desc)) }
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
    GLStateManager::Get().NotifyTextureRelease(id_, GLStateManager::GetTextureTarget(GetType()));
}

void GLTexture::SetName(const char* name)
{
    GLSetObjectLabel(GL_TEXTURE, GetID(), name);
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

Extent3D GLTexture::GetMipExtent(std::uint32_t mipLevel) const
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
        GLStateManager::Get().PushBoundTexture(GLStateManager::GetTextureTarget(GetType()));
        {
            /* Bind texture and query attributes */
            GLStateManager::Get().BindGLTexture(*this);
            auto target = GLGetTextureParamTarget(GetType());
            glGetTexLevelParameteriv(target, level, GL_TEXTURE_WIDTH,  &texSize[0]);
            glGetTexLevelParameteriv(target, level, GL_TEXTURE_HEIGHT, &texSize[1]);
            glGetTexLevelParameteriv(target, level, GL_TEXTURE_DEPTH,  &texSize[2]);
        }
        GLStateManager::Get().PopBoundTexture();
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

TextureDescriptor GLTexture::GetDesc() const
{
    TextureDescriptor texDesc;

    texDesc.type            = GetType();
    texDesc.bindFlags       = 0;
    texDesc.cpuAccessFlags  = 0;
    texDesc.mipLevels       = static_cast<std::uint32_t>(GetNumMipLevels());

    /* Query hardware texture format and size */
    GLint internalFormat = 0, extent[3] = { 0 };
    GetTexParams(&internalFormat, extent);

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

#ifdef GL_ARB_copy_image

static void GLCopyImageSubData(
    GLTexture&      dstTexture,
    GLint           dstLevel,
    const Offset3D& dstOffset,
    GLTexture&      srcTexture,
    GLint           srcLevel,
    const Offset3D& srcOffset,
    const Extent3D& extent)
{
    /* Copy raw data of texture directly (GL 4.3+) */
    glCopyImageSubData(
        srcTexture.GetID(),
        GLTypes::Map(srcTexture.GetType()),
        srcLevel,
        srcOffset.x,
        srcOffset.y,
        srcOffset.z,
        dstTexture.GetID(),
        GLTypes::Map(dstTexture.GetType()),
        dstLevel,
        dstOffset.x,
        dstOffset.y,
        dstOffset.z,
        static_cast<GLsizei>(extent.width),
        static_cast<GLsizei>(extent.height),
        static_cast<GLsizei>(extent.depth)
    );
}

#endif // /GL_ARB_copy_image

static void GLCopyTexSubImage(
    GLTexture&      dstTexture,
    GLint           dstLevel,
    const Offset3D& dstOffset,
    GLTexture&      srcTexture,
    GLint           srcLevel,
    Offset3D        srcOffset,
    const Extent3D& extent)
{
    const TextureType       type        = dstTexture.GetType();
    const GLTextureTarget   target      = GLStateManager::GetTextureTarget(type);
    const GLenum            targetGL    = GLTypes::Map(type);

    /* Store bound texture and framebuffer */
    GLStateManager::Get().PushBoundTexture(target);
    GLStateManager::Get().PushBoundFramebuffer(GLFramebufferTarget::READ_FRAMEBUFFER);
    {
        /* Create temporary FBO for source texture to read from GL_READ_FRAMEBUFFER in copy texture operator */
        GLReadTextureFBO readFBO;
        GLStateManager::Get().BindTexture(target, dstTexture.GetID());

        switch (type)
        {
            case TextureType::Texture1D:
            {
                readFBO.Attach(srcTexture, srcLevel, srcOffset);
                glCopyTexSubImage1D(
                    targetGL,
                    dstLevel,
                    dstOffset.x,
                    srcOffset.x,
                    0,
                    static_cast<GLsizei>(extent.width)
                );
            }
            break;

            case TextureType::Texture1DArray:
            case TextureType::Texture2D:
            case TextureType::Texture2DMS:
            {
                readFBO.Attach(srcTexture, srcLevel, srcOffset);
                glCopyTexSubImage2D(
                    targetGL,
                    dstLevel,
                    dstOffset.x,
                    dstOffset.y,
                    srcOffset.x,
                    srcOffset.y,
                    static_cast<GLsizei>(extent.width),
                    static_cast<GLsizei>(extent.height)
                );
            }
            break;

            case TextureType::Texture3D:
            case TextureType::Texture2DArray:
            case TextureType::Texture2DMSArray:
            case TextureType::TextureCube:
            case TextureType::TextureCubeArray:
            {
                for (decltype(extent.depth) i = 0; i < extent.depth; ++i)
                {
                    readFBO.Attach(srcTexture, srcLevel, srcOffset);
                    glCopyTexSubImage3D(
                        targetGL,
                        dstLevel,
                        dstOffset.x,
                        dstOffset.y,
                        dstOffset.z + i,
                        srcOffset.x,
                        srcOffset.y,
                        static_cast<GLsizei>(extent.width),
                        static_cast<GLsizei>(extent.height)
                    );
                    srcOffset.z++;
                }
            }
            break;
        }
    }
    /* Restore previous bound texture and framebuffer */
    GLStateManager::Get().PopBoundFramebuffer();
    GLStateManager::Get().PopBoundTexture();
}

void GLTexture::CopyImageSubData(
    GLint           dstLevel,
    const Offset3D& dstOffset,
    GLTexture&      srcTexture,
    GLint           srcLevel,
    const Offset3D& srcOffset,
    const Extent3D& extent)
{
    #ifdef GL_ARB_copy_image
    if (HasExtension(GLExt::ARB_copy_image))
    {
        /* Copy raw data of texture directly (GL 4.3+) */
        GLCopyImageSubData(*this, dstLevel, dstOffset, srcTexture, srcLevel, srcOffset, extent);
    }
    else
    #endif // /GL_ARB_copy_image
    {
        /* Copy source texture from GL_READ_BUFFER into destination texture */
        GLCopyTexSubImage(*this, dstLevel, dstOffset, srcTexture, srcLevel, srcOffset, extent);
    }
}

void GLTexture::TextureView(GLTexture& sharedTexture, const TextureViewDescriptor& textureViewDesc)
{
    #ifdef GL_ARB_texture_view
    if (HasExtension(GLExt::ARB_texture_view))
    {
        /* Initialize texture with texture-view description */
        glTextureView(
            GetID(),
            GLTypes::Map(textureViewDesc.type),
            sharedTexture.GetID(),
            GLTypes::Map(textureViewDesc.format),
            textureViewDesc.subresource.baseMipLevel,
            textureViewDesc.subresource.numMipLevels,
            textureViewDesc.subresource.baseArrayLayer,
            textureViewDesc.subresource.numArrayLayers
        );
    }
    #endif // /GL_ARB_texture_view
}

GLenum GLTexture::GetInternalFormat() const
{
    /* Query hardware texture format */
    GLint internalFormat = 0;
    GetTexParams(&internalFormat, nullptr);
    return static_cast<GLenum>(internalFormat);
}


/*
 * ======= Private: =======
 */

void GLTexture::GetTexParams(GLint* internalFormat, GLint* extent) const
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
        GLStateManager::Get().PushBoundTexture(GLStateManager::GetTextureTarget(GetType()));
        {
            /* Bind texture and query attributes */
            GLStateManager::Get().BindGLTexture(*this);
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
        GLStateManager::Get().PopBoundTexture();
    }
}


} // /namespace LLGL



// ================================================================================
