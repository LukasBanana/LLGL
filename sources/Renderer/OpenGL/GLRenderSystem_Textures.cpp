/*
 * GLRenderSystem_Textures.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLRenderSystem.h"
#include "../GLCommon/GLTypes.h"
#include "../GLCommon/Texture/GLTexImage.h"
#include "../GLCommon/Texture/GLTexSubImage.h"
#include "Ext/GLExtensions.h"
#include "../CheckedCast.h"
#include "../../Core/Helper.h"
#include "../Assertion.h"


namespace LLGL
{


/* ----- Textures ----- */

Texture* GLRenderSystem::CreateTexture(const TextureDescriptor& textureDesc, const ImageDescriptor* imageDesc)
{
    auto texture = MakeUnique<GLTexture>(textureDesc.type);

    /* Bind texture */
    GLStateManager::active->BindTexture(*texture);

    /* Initialize texture parameters for the first time */
    auto target = GLTypes::Map(textureDesc.type);
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
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
            LLGL_ASSERT_CAP(has3DTextures);
            GLTexImage3D(textureDesc, imageDesc);
            break;

        case TextureType::TextureCube:
            LLGL_ASSERT_CAP(hasCubeTextures);
            GLTexImageCube(textureDesc, imageDesc);
            break;

        case TextureType::Texture1DArray:
            LLGL_ASSERT_CAP(hasTextureArrays);
            GLTexImage1DArray(textureDesc, imageDesc);
            break;

        case TextureType::Texture2DArray:
            LLGL_ASSERT_CAP(hasTextureArrays);
            GLTexImage2DArray(textureDesc, imageDesc);
            break;

        case TextureType::TextureCubeArray:
            LLGL_ASSERT_CAP(hasCubeTextureArrays);
            GLTexImageCubeArray(textureDesc, imageDesc);
            break;

        case TextureType::Texture2DMS:
            LLGL_ASSERT_CAP(hasMultiSampleTextures);
            GLTexImage2DMS(textureDesc);
            break;

        case TextureType::Texture2DMSArray:
            LLGL_ASSERT_CAP(hasMultiSampleTextures);
            GLTexImage2DMSArray(textureDesc);
            break;

        default:
            throw std::invalid_argument("failed to create texture with invalid texture type");
            break;
    }

    return TakeOwnership(textures_, std::move(texture));
}

TextureArray* GLRenderSystem::CreateTextureArray(std::uint32_t numTextures, Texture* const * textureArray)
{
    AssertCreateTextureArray(numTextures, textureArray);
    return TakeOwnership(textureArrays_, MakeUnique<GLTextureArray>(numTextures, textureArray));
}

void GLRenderSystem::Release(Texture& texture)
{
    /* Notify GL state manager about object release, then release object */
    auto& textureGL = LLGL_CAST(GLTexture&, texture);
    GLStateManager::NotifyTextureRelease(textureGL.GetID(), GLStateManager::GetTextureTarget(textureGL.GetType()));
    RemoveFromUniqueSet(textures_, &texture);
}

void GLRenderSystem::Release(TextureArray& textureArray)
{
    /* Notify GL state manager about object release, then release object */
    auto& textureArrayGL = LLGL_CAST(GLTextureArray&, textureArray);

    const auto& texIDs      = textureArrayGL.GetIDArray();
    const auto& texTargets  = textureArrayGL.GetTargetArray();

    for (std::size_t i = 0, n = texIDs.size(); i < n; ++i)
        GLStateManager::NotifyTextureRelease(texIDs[i], texTargets[i]);

    RemoveFromUniqueSet(textureArrays_, &textureArray);
}

/* ----- "WriteTexture..." functions ----- */

void GLRenderSystem::WriteTexture(Texture& texture, const SubTextureDescriptor& subTextureDesc, const ImageDescriptor& imageDesc)
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
            LLGL_ASSERT_CAP(has3DTextures);
            GLTexSubImage3D(subTextureDesc, imageDesc);
            break;

        case TextureType::TextureCube:
            LLGL_ASSERT_CAP(hasCubeTextures);
            GLTexSubImageCube(subTextureDesc, imageDesc);
            break;

        case TextureType::Texture1DArray:
            LLGL_ASSERT_CAP(hasTextureArrays);
            GLTexSubImage1DArray(subTextureDesc, imageDesc);
            break;

        case TextureType::Texture2DArray:
            LLGL_ASSERT_CAP(hasTextureArrays);
            GLTexSubImage2DArray(subTextureDesc, imageDesc);
            break;

        case TextureType::TextureCubeArray:
            LLGL_ASSERT_CAP(hasCubeTextureArrays);
            GLTexSubImageCubeArray(subTextureDesc, imageDesc);
            break;

        default:
            break;
    }
}

void GLRenderSystem::ReadTexture(const Texture& texture, std::uint32_t mipLevel, ImageFormat imageFormat, DataType dataType, void* data, std::size_t dataSize)
{
    LLGL_ASSERT_PTR(data);

    auto& textureGL = LLGL_CAST(const GLTexture&, texture);

    /* Read image data from texture */
    #if defined GL_ARB_direct_state_access && defined LLGL_GL_ENABLE_DSA_EXT
    if (HasExtension(GLExt::ARB_direct_state_access))
    {
        glGetTextureImage(
            textureGL.GetID(),
            static_cast<GLint>(mipLevel),
            GLTypes::Map(imageFormat),
            GLTypes::Map(dataType),
            static_cast<GLsizei>(dataSize),
            data
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
            GLTypes::Map(imageFormat),
            GLTypes::Map(dataType),
            data
        );
    }
}

void GLRenderSystem::GenerateMips(Texture& texture)
{
    /* Bind texture to active layer */
    auto& textureGL = LLGL_CAST(GLTexture&, texture);

    /* Generate MIP-maps and update texture minification filter to a default value */
    #if defined GL_ARB_direct_state_access && defined LLGL_GL_ENABLE_DSA_EXT
    if (HasExtension(GLExt::ARB_direct_state_access))
    {
        glGenerateTextureMipmap(textureGL.GetID());
        glTextureParameteri(textureGL.GetID(), GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    }
    else
    #endif
    {
        GLStateManager::active->BindTexture(textureGL);
        auto target = GLTypes::Map(textureGL.GetType());
        glGenerateMipmap(target);
        glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    }
}


} // /namespace LLGL



// ================================================================================
