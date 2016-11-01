/*
 * GLRenderSystem_Textures.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
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
            GLBuildTexture1D(textureDesc, imageDesc, GetConfiguration().defaultImageColor);
            break;

        case TextureType::Texture2D:
            GLBuildTexture2D(textureDesc, imageDesc, GetConfiguration().defaultImageColor);
            break;

        case TextureType::Texture3D:
            LLGL_ASSERT_CAP(has3DTextures);
            GLBuildTexture3D(textureDesc, imageDesc, GetConfiguration().defaultImageColor);
            break;

        case TextureType::TextureCube:
            LLGL_ASSERT_CAP(hasCubeTextures);
            GLBuildTextureCube(textureDesc, imageDesc, GetConfiguration().defaultImageColor);
            break;

        case TextureType::Texture1DArray:
            LLGL_ASSERT_CAP(hasTextureArrays);
            GLBuildTexture1DArray(textureDesc, imageDesc, GetConfiguration().defaultImageColor);
            break;

        case TextureType::Texture2DArray:
            LLGL_ASSERT_CAP(hasTextureArrays);
            GLBuildTexture2DArray(textureDesc, imageDesc, GetConfiguration().defaultImageColor);
            break;

        case TextureType::TextureCubeArray:
            LLGL_ASSERT_CAP(hasCubeTextureArrays);
            GLBuildTextureCubeArray(textureDesc, imageDesc, GetConfiguration().defaultImageColor);
            break;

        case TextureType::Texture2DMS:
            LLGL_ASSERT_CAP(hasMultiSampleTextures);
            GLBuildTexture2DMS(textureDesc);
            break;

        case TextureType::Texture2DMSArray:
            LLGL_ASSERT_CAP(hasMultiSampleTextures);
            GLBuildTexture2DMSArray(textureDesc);
            break;

        default:
            throw std::invalid_argument("failed to create texture with invalid texture type");
            break;
    }

    return TakeOwnership(textures_, std::move(texture));
}

TextureArray* GLRenderSystem::CreateTextureArray(unsigned int numTextures, Texture* const * textureArray)
{
    AssertCreateTextureArray(numTextures, textureArray);
    return TakeOwnership(textureArrays_, MakeUnique<GLTextureArray>(numTextures, textureArray));
}

void GLRenderSystem::Release(Texture& texture)
{
    RemoveFromUniqueSet(textures_, &texture);
}

void GLRenderSystem::Release(TextureArray& textureArray)
{
    RemoveFromUniqueSet(textureArrays_, &textureArray);
}

TextureDescriptor GLRenderSystem::QueryTextureDescriptor(const Texture& texture)
{
    /* Bind texture */
    auto& textureGL = LLGL_CAST(const GLTexture&, texture);
    GLStateManager::active->BindTexture(textureGL);

    /* Setup texture descriptor */
    TextureDescriptor desc;

    desc.type = texture.GetType();

    auto target = GLTypes::Map(texture.GetType());

    /* Query hardware texture format */
    GLint internalFormat = 0;
    glGetTexLevelParameteriv(target, 0, GL_TEXTURE_INTERNAL_FORMAT, &internalFormat);
    GLTypes::Unmap(desc.format, static_cast<GLenum>(internalFormat));

    /* Query texture size */
    GLint texSize[3] = { 0 };
    glGetTexLevelParameteriv(target, 0, GL_TEXTURE_WIDTH,  &texSize[0]);
    glGetTexLevelParameteriv(target, 0, GL_TEXTURE_HEIGHT, &texSize[1]);
    glGetTexLevelParameteriv(target, 0, GL_TEXTURE_DEPTH,  &texSize[2]);

    desc.texture3D.width    = static_cast<unsigned int>(texSize[0]);
    desc.texture3D.height   = static_cast<unsigned int>(texSize[1]);
    desc.texture3D.depth    = static_cast<unsigned int>(texSize[2]);

    if (desc.type == TextureType::TextureCube || desc.type == TextureType::TextureCubeArray)
        desc.texture3D.depth /= 6;

    return desc;
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
            GLWriteTexture1D(subTextureDesc, imageDesc);
            break;

        case TextureType::Texture2D:
            GLWriteTexture2D(subTextureDesc, imageDesc);
            break;

        case TextureType::Texture3D:
            LLGL_ASSERT_CAP(has3DTextures);
            GLWriteTexture3D(subTextureDesc, imageDesc);
            break;

        case TextureType::TextureCube:
            LLGL_ASSERT_CAP(hasCubeTextures);
            GLWriteTextureCube(subTextureDesc, imageDesc);
            break;

        case TextureType::Texture1DArray:
            LLGL_ASSERT_CAP(hasTextureArrays);
            GLWriteTexture1DArray(subTextureDesc, imageDesc);
            break;

        case TextureType::Texture2DArray:
            LLGL_ASSERT_CAP(hasTextureArrays);
            GLWriteTexture2DArray(subTextureDesc, imageDesc);
            break;

        case TextureType::TextureCubeArray:
            LLGL_ASSERT_CAP(hasCubeTextureArrays);
            GLWriteTextureCubeArray(subTextureDesc, imageDesc);
            break;

        default:
            break;
    }
}

void GLRenderSystem::ReadTexture(const Texture& texture, int mipLevel, ImageFormat imageFormat, DataType dataType, void* buffer)
{
    LLGL_ASSERT_PTR(buffer);

    /* Bind texture */
    auto& textureGL = LLGL_CAST(const GLTexture&, texture);
    GLStateManager::active->BindTexture(textureGL);

    /* Read image data from texture */
    glGetTexImage(
        GLTypes::Map(textureGL.GetType()),
        mipLevel,
        GLTypes::Map(imageFormat),
        GLTypes::Map(dataType),
        buffer
    );
}

void GLRenderSystem::GenerateMips(Texture& texture)
{
    /* Bind texture to active layer */
    auto& textureGL = LLGL_CAST(GLTexture&, texture);
    GLStateManager::active->BindTexture(textureGL);

    auto target = GLTypes::Map(textureGL.GetType());

    /* Generate MIP-maps */
    glGenerateMipmap(target);

    /* Update texture minification filter to a default value */
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
}


} // /namespace LLGL



// ================================================================================
