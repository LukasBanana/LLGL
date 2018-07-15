/*
 * GLRenderSystem_Textures.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLRenderSystem.h"
#include "../GLCommon/GLTypes.h"
#include "../GLCommon/Texture/GLTexImage.h"
#include "../GLCommon/Texture/GLTexSubImage.h"
#include "Ext/GLExtensions.h"
#include "../CheckedCast.h"
#include "../../Core/Helper.h"
#include "../../Core/Assertion.h"


namespace LLGL
{


/* ----- Textures ----- */

static GLint GetGlTextureMinFilter(const TextureDescriptor& textureDesc)
{
    if (IsMipMappedTexture(textureDesc))
        return GL_LINEAR_MIPMAP_LINEAR;
    else
        return GL_LINEAR;
}

Texture* GLRenderSystem::CreateTexture(const TextureDescriptor& textureDesc, const SrcImageDescriptor* imageDesc)
{
    auto texture = MakeUnique<GLTexture>(textureDesc.type);

    /* Bind texture */
    GLStateManager::active->BindTexture(*texture);

    /* Initialize texture parameters for the first time */
    auto target = GLTypes::Map(textureDesc.type);

    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GetGlTextureMinFilter(textureDesc));
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    /* Build texture storage and upload image dataa */
    switch (textureDesc.type)
    {
        case TextureType::Texture1D:
            GLTexImage1D(textureDesc, imageDesc);
            break;

        case TextureType::Texture2D:
            GLTexImage2D(textureDesc, imageDesc);
            break;

        case TextureType::Texture3D:
            LLGL_ASSERT_FEATURE_SUPPORT(has3DTextures);
            GLTexImage3D(textureDesc, imageDesc);
            break;

        case TextureType::TextureCube:
            LLGL_ASSERT_FEATURE_SUPPORT(hasCubeTextures);
            GLTexImageCube(textureDesc, imageDesc);
            break;

        case TextureType::Texture1DArray:
            LLGL_ASSERT_FEATURE_SUPPORT(hasArrayTextures);
            GLTexImage1DArray(textureDesc, imageDesc);
            break;

        case TextureType::Texture2DArray:
            LLGL_ASSERT_FEATURE_SUPPORT(hasArrayTextures);
            GLTexImage2DArray(textureDesc, imageDesc);
            break;

        case TextureType::TextureCubeArray:
            LLGL_ASSERT_FEATURE_SUPPORT(hasCubeArrayTextures);
            GLTexImageCubeArray(textureDesc, imageDesc);
            break;

        case TextureType::Texture2DMS:
            LLGL_ASSERT_FEATURE_SUPPORT(hasMultiSampleTextures);
            GLTexImage2DMS(textureDesc);
            break;

        case TextureType::Texture2DMSArray:
            LLGL_ASSERT_FEATURE_SUPPORT(hasMultiSampleTextures);
            GLTexImage2DMSArray(textureDesc);
            break;

        default:
            throw std::invalid_argument("failed to create texture with invalid texture type");
            break;
    }

    return TakeOwnership(textures_, std::move(texture));
}

void GLRenderSystem::Release(Texture& texture)
{
    RemoveFromUniqueSet(textures_, &texture);
}

/* ----- "WriteTexture..." functions ----- */

void GLRenderSystem::WriteTexture(Texture& texture, const SubTextureDescriptor& subTextureDesc, const SrcImageDescriptor& imageDesc)
{
    /* Bind texture and write texture sub data */
    auto& textureGL = LLGL_CAST(GLTexture&, texture);
    GLStateManager::active->BindTexture(textureGL);

    /* Write data into specific texture type */
    switch (texture.GetType())
    {
        case TextureType::Texture1D:
            GLTexSubImage1D(subTextureDesc, imageDesc);
            break;

        case TextureType::Texture2D:
            GLTexSubImage2D(subTextureDesc, imageDesc);
            break;

        case TextureType::Texture3D:
            LLGL_ASSERT_FEATURE_SUPPORT(has3DTextures);
            GLTexSubImage3D(subTextureDesc, imageDesc);
            break;

        case TextureType::TextureCube:
            LLGL_ASSERT_FEATURE_SUPPORT(hasCubeTextures);
            GLTexSubImageCube(subTextureDesc, imageDesc);
            break;

        case TextureType::Texture1DArray:
            LLGL_ASSERT_FEATURE_SUPPORT(hasArrayTextures);
            GLTexSubImage1DArray(subTextureDesc, imageDesc);
            break;

        case TextureType::Texture2DArray:
            LLGL_ASSERT_FEATURE_SUPPORT(hasArrayTextures);
            GLTexSubImage2DArray(subTextureDesc, imageDesc);
            break;

        case TextureType::TextureCubeArray:
            LLGL_ASSERT_FEATURE_SUPPORT(hasCubeArrayTextures);
            GLTexSubImageCubeArray(subTextureDesc, imageDesc);
            break;

        default:
            break;
    }
}

void GLRenderSystem::ReadTexture(const Texture& texture, std::uint32_t mipLevel, const DstImageDescriptor& imageDesc)
{
    LLGL_ASSERT_PTR(imageDesc.data);

    auto& textureGL = LLGL_CAST(const GLTexture&, texture);

    /* Read image data from texture */
    #if defined GL_ARB_direct_state_access && defined LLGL_GL_ENABLE_DSA_EXT
    if (HasExtension(GLExt::ARB_direct_state_access))
    {
        glGetTextureImage(
            textureGL.GetID(),
            static_cast<GLint>(mipLevel),
            GLTypes::Map(imageDesc.format),
            GLTypes::Map(imageDesc.dataType),
            static_cast<GLsizei>(imageDesc.dataSize),
            imageDesc.data
        );
    }
    else
    #endif
    {
        /* Bind texture and read image data from texture */
        GLStateManager::active->BindTexture(textureGL);
        glGetTexImage(
            GLTypes::Map(textureGL.GetType()),
            static_cast<GLint>(mipLevel),
            GLTypes::Map(imageDesc.format),
            GLTypes::Map(imageDesc.dataType),
            imageDesc.data
        );
    }
}

void GLRenderSystem::GenerateMips(Texture& texture)
{
    auto& textureGL = LLGL_CAST(GLTexture&, texture);
    GenerateMipsPrimary(textureGL.GetID(), textureGL.GetType());
}

void GLRenderSystem::GenerateMips(Texture& texture, std::uint32_t baseMipLevel, std::uint32_t numMipLevels, std::uint32_t baseArrayLayer, std::uint32_t numArrayLayers)
{
    if (numMipLevels > 0 && numArrayLayers > 0)
    {
        #ifdef LLGL_ENABLE_CUSTOM_SUB_MIPGEN

        if (texture.GetType() == TextureType::Texture3D)
        {
            /* Generate MIP-maps in default process */
            GLRenderSystem::GenerateMips(texture);
        }
        else
        {
            /* Generate MIP-maps in custom sub generation process */
            auto& textureGL = LLGL_CAST(GLTexture&, texture);
            auto extent = textureGL.QueryMipExtent(baseMipLevel);

            GenerateSubMipsWithFBO(
                textureGL,
                extent,
                static_cast<GLint>(baseMipLevel),
                static_cast<GLint>(numMipLevels),
                static_cast<GLint>(baseArrayLayer),
                static_cast<GLint>(numArrayLayers)
            );
        }

        #else

        #ifdef GL_ARB_texture_view
        if (HasExtension(GLExt::ARB_texture_view))
        {
            /* Generate MIP-maps in GL_ARB_texture_view extension process */
            auto& textureGL = LLGL_CAST(GLTexture&, texture);

            GenerateSubMipsWithTextureView(
                textureGL,
                static_cast<GLuint>(baseMipLevel),
                static_cast<GLuint>(numMipLevels),
                static_cast<GLuint>(baseArrayLayer),
                static_cast<GLuint>(numArrayLayers)
            );
        }
        else
        #endif
        {
            /* Generate MIP-maps in default process */
            GLRenderSystem::GenerateMips(texture);
        }

        #endif // /LLGL_ENABLE_CUSTOM_SUB_MIPGEN
    }
}


/*
 * ======= Private: =======
 */

void GLRenderSystem::GenerateMipsPrimary(GLuint texID, const TextureType texType)
{
    #if defined GL_ARB_direct_state_access && defined LLGL_GL_ENABLE_DSA_EXT
    if (HasExtension(GLExt::ARB_direct_state_access))
    {
        /* Generate MIP-maps of named texture object */
        glGenerateTextureMipmap(texID);
    }
    else
    #endif
    {
        /* Restore previously bound texture on active layer */
        auto texTarget = GLStateManager::GetTextureTarget(texType);
        GLStateManager::active->PushBoundTexture(texTarget);
        {
            /* Bind texture and generate MIP-maps */
            GLStateManager::active->BindTexture(texTarget, texID);
            glGenerateMipmap(GLTypes::Map(texType));
        }
        GLStateManager::active->PopBoundTexture();
    }
}

#ifdef LLGL_ENABLE_CUSTOM_SUB_MIPGEN

static void GetNextMipSize(GLint& s)
{
    s = std::max(1u, s / 2u);
}

static void BlitFramebufferLinear(GLint srcWidth, GLint srcHeight, GLint dstWidth, GLint dstHeight)
{
    glBlitFramebuffer(0, 0, srcWidth, srcHeight, 0, 0, dstWidth, dstHeight, GL_COLOR_BUFFER_BIT, GL_LINEAR);
}

static void GenerateSubMipsTexture1D(const Extent3D& extent, GLuint texID, GLint baseMipLevel, GLint numMipLevels)
{
    /* Get extent of base MIP level */
    auto srcWidth = static_cast<GLint>(extent.width);

    auto dstWidth = srcWidth;

    /* Blit current MIP level into next MIP level with linear sampling filter */
    for (auto mipLevel = baseMipLevel; mipLevel + 1 < baseMipLevel + numMipLevels; ++mipLevel)
    {
        GetNextMipSize(dstWidth);

        glFramebufferTexture1D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_1D, texID, mipLevel);
        glFramebufferTexture1D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_1D, texID, mipLevel + 1);

        BlitFramebufferLinear(srcWidth, 1, dstWidth, 1);

        srcWidth = dstWidth;
    }
}

static void GenerateSubMipsTexture2D(const Extent3D& extent, GLuint texID, GLenum texTarget, GLint baseMipLevel, GLint numMipLevels)
{
    /* Get extent of base MIP level */
    auto srcWidth   = static_cast<GLint>(extent.width);
    auto srcHeight  = static_cast<GLint>(extent.height);

    auto dstWidth   = srcWidth;
    auto dstHeight  = srcHeight;

    /* Blit current MIP level into next MIP level with linear sampling filter */
    for (auto mipLevel = baseMipLevel; mipLevel + 1 < baseMipLevel + numMipLevels; ++mipLevel)
    {
        GetNextMipSize(dstWidth);
        GetNextMipSize(dstHeight);

        glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texTarget, texID, mipLevel);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texTarget, texID, mipLevel + 1);

        BlitFramebufferLinear(srcWidth, srcHeight, dstWidth, dstHeight);

        srcWidth    = dstWidth;
        srcHeight   = dstHeight;
    }
}

static void GenerateSubMipsTextureLayer(const Extent3D& extent, GLuint texID, GLint baseMipLevel, GLint numMipLevels, GLint arrayLayer)
{
    /* Get extent of base MIP level */
    auto srcWidth   = static_cast<GLint>(extent.width);
    auto srcHeight  = static_cast<GLint>(extent.height);

    auto dstWidth   = srcWidth;
    auto dstHeight  = srcHeight;

    /* Blit current MIP level into next MIP level with linear sampling filter */
    for (auto mipLevel = baseMipLevel; mipLevel + 1 < baseMipLevel + numMipLevels; ++mipLevel)
    {
        GetNextMipSize(dstWidth);
        GetNextMipSize(dstHeight);

        glFramebufferTextureLayer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texID, mipLevel, arrayLayer);
        glFramebufferTextureLayer(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texID, mipLevel + 1, arrayLayer);

        BlitFramebufferLinear(srcWidth, srcHeight, dstWidth, dstHeight);

        srcWidth    = dstWidth;
        srcHeight   = dstHeight;
    }
}

void GLRenderSystem::GenerateSubMipsWithFBO(GLTexture& textureGL, const Extent3D& extent, GLint baseMipLevel, GLint numMipLevels, GLint baseArrayLayer, GLint numArrayLayers)
{
    /* Get GL texture ID and texture target */
    auto texID      = textureGL.GetID();
    auto texType    = textureGL.GetType();
    auto texTarget  = GLTypes::Map(texType);

    mipGenerationFBOPair_.CreateFBOs();

    GLStateManager::active->PushBoundFramebuffer(GLFramebufferTarget::READ_FRAMEBUFFER);
    GLStateManager::active->PushBoundFramebuffer(GLFramebufferTarget::DRAW_FRAMEBUFFER);
    {
        /* Bind read framebuffer for <current> MIP level, and draw framebuffer for <next> MIP level */
        GLStateManager::active->BindFramebuffer(GLFramebufferTarget::READ_FRAMEBUFFER, mipGenerationFBOPair_.fbos[0]);
        GLStateManager::active->BindFramebuffer(GLFramebufferTarget::DRAW_FRAMEBUFFER, mipGenerationFBOPair_.fbos[1]);

        switch (texType)
        {
            case TextureType::Texture1D:
            {
                GenerateSubMipsTexture1D(extent, texID, baseMipLevel, numMipLevels);
            }
            break;

            case TextureType::Texture2D:
            case TextureType::Texture2DMS:
            {
                GenerateSubMipsTexture2D(extent, texID, texTarget, baseMipLevel, numMipLevels);
            }
            break;

            case TextureType::Texture3D:
            break;

            case TextureType::TextureCube:
            {
                /* Generate MIP-maps for all 6 cube faces */
                static const GLenum g_cubeFaceTexTargets[] =
                {
                    GL_TEXTURE_CUBE_MAP_POSITIVE_X,
                    GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
                    GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
                    GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
                    GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
                    GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
                };

                for (std::size_t i = 0; i < 6; ++i)
                    GenerateSubMipsTexture2D(extent, texID, g_cubeFaceTexTargets[i], baseMipLevel, numMipLevels);
            }
            break;

            case TextureType::Texture1DArray:
            case TextureType::Texture2DArray:
            case TextureType::Texture2DMSArray:
            {
                /* Generate MIP-maps for each specified array layer */
                for (auto arrayLayer = baseArrayLayer; arrayLayer < baseArrayLayer + numArrayLayers; ++arrayLayer)
                    GenerateSubMipsTextureLayer(extent, texID, baseMipLevel, numMipLevels, arrayLayer);
            }
            break;

            case TextureType::TextureCubeArray:
            {
                /* Generate MIP-maps of all 6 cube faces */
                baseArrayLayer *= 6;
                numArrayLayers *= 6;

                /* Generate MIP-maps for each specified array layer */
                for (auto arrayLayer = baseArrayLayer; arrayLayer < baseArrayLayer + numArrayLayers; ++arrayLayer)
                    GenerateSubMipsTextureLayer(extent, texID, baseMipLevel, numMipLevels, arrayLayer);
            }
            break;
        }
    }
    GLStateManager::active->PopBoundFramebuffer();
    GLStateManager::active->PopBoundFramebuffer();
}

#else

void GLRenderSystem::GenerateSubMipsWithFBO(GLTexture& textureGL, const Extent3D& extent, GLint baseMipLevel, GLint numMipLevels, GLint baseArrayLayer, GLint numArrayLayers)
{
    // dummy
}

#endif // /LLGL_ENABLE_CUSTOM_SUB_MIPGEN

#ifdef GL_ARB_texture_view

void GLRenderSystem::GenerateSubMipsWithTextureView(GLTexture& textureGL, GLuint baseMipLevel, GLuint numMipLevels, GLuint baseArrayLayer, GLuint numArrayLayers)
{
    /* Get GL texture ID and texture target */
    auto texID          = textureGL.GetID();
    auto texType        = textureGL.GetType();
    auto texTarget      = GLTypes::Map(texType);
    auto internalFormat = textureGL.QueryGLInternalFormat();

    /* Generate new texture to be used as view (due to immutable storage) */
    GLuint texViewID = 0;
    glGenTextures(1, &texViewID);

    /*
    Create texture view as storage alias from the specified input texture.
    Note: texture views can only be created with textures that have been allocated with glTexStorage!
    see https://www.khronos.org/registry/OpenGL/extensions/ARB/ARB_texture_view.txt
    */
    glTextureView(texViewID, texTarget, texID, internalFormat, baseMipLevel, numMipLevels, baseArrayLayer, numArrayLayers);

    /* Generate MIP-maps for texture view */
    GenerateMipsPrimary(texViewID, texType);

    /* Release temporary texture view */
    glDeleteTextures(1, &texViewID);
}

#else

void GLRenderSystem::GenerateSubMipsWithTextureView(GLTexture& textureGL, GLuint baseMipLevel, GLuint numMipLevels, GLuint baseArrayLayer, GLuint numArrayLayers)
{
    // dummy
}

#endif // /GL_ARB_texture_view


} // /namespace LLGL



// ================================================================================
