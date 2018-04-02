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

TextureDescriptor GLRenderSystem::QueryTextureDescriptor(const Texture& texture)
{
    auto& textureGL = LLGL_CAST(const GLTexture&, texture);

    TextureDescriptor desc;

    desc.type = texture.GetType();

    /* Query hardware texture format and size */
    GLint internalFormat = 0, texSize[3] = { 0 };

    #if defined GL_ARB_direct_state_access && defined LLGL_GL_ENABLE_DSA_EXT
    if (HasExtension(GLExt::ARB_direct_state_access))
    {
        auto texID = textureGL.GetID();
        glGetTextureLevelParameteriv(texID, 0, GL_TEXTURE_INTERNAL_FORMAT, &internalFormat);
        glGetTextureLevelParameteriv(texID, 0, GL_TEXTURE_WIDTH, &texSize[0]);
        glGetTextureLevelParameteriv(texID, 0, GL_TEXTURE_HEIGHT, &texSize[1]);
        glGetTextureLevelParameteriv(texID, 0, GL_TEXTURE_DEPTH, &texSize[2]);
    }
    else
    #endif
    {
        GLStateManager::active->BindTexture(textureGL);
        auto target = GLTypes::Map(texture.GetType());
        glGetTexLevelParameteriv(target, 0, GL_TEXTURE_INTERNAL_FORMAT, &internalFormat);
        glGetTexLevelParameteriv(target, 0, GL_TEXTURE_WIDTH, &texSize[0]);
        glGetTexLevelParameteriv(target, 0, GL_TEXTURE_HEIGHT, &texSize[1]);
        glGetTexLevelParameteriv(target, 0, GL_TEXTURE_DEPTH, &texSize[2]);
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

void GLRenderSystem::ReadTexture(const Texture& texture, int mipLevel, ImageFormat imageFormat, DataType dataType, void* buffer)
{
    LLGL_ASSERT_PTR(buffer);

    auto& textureGL = LLGL_CAST(const GLTexture&, texture);

    /* Read image data from texture */
    #if 0
    #if defined GL_ARB_direct_state_access && defined LLGL_GL_ENABLE_DSA_EXT
    if (HasExtension(GLExt::ARB_direct_state_access))
    {
        //glGetTextureImage(textureGL.GetID(), mipLevel, GLTypes::Map(imageFormat), GLTypes::Map(dataType), bufferSize, buffer);
    }
    else
    #endif
    #endif
    {
        /* Bind texture and read image data from texture */
        GLStateManager::active->BindTexture(textureGL);
        glGetTexImage(GLTypes::Map(textureGL.GetType()), mipLevel, GLTypes::Map(imageFormat), GLTypes::Map(dataType), buffer);
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
