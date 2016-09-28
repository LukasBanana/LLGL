/*
 * GLRenderSystem_Textures.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLRenderSystem.h"
#include "GLTypes.h"
#include "Ext/GLExtensions.h"
#include "../CheckedCast.h"
#include "../../Core/Helper.h"
#include "../Assertion.h"


namespace LLGL
{


/* ----- Textures ----- */

Texture* GLRenderSystem::CreateTexture(const TextureDescriptor& textureDesc, const ImageDescriptor* imageDesc)
{
    auto texture = MakeUnique<GLTexture>();

    BindTextureAndSetType(*texture, textureDesc.type);

    switch (textureDesc.type)
    {
        case TextureType::Texture1D:
            BuildTexture1D(textureDesc, imageDesc);
            break;
        case TextureType::Texture2D:
            BuildTexture2D(textureDesc, imageDesc);
            break;
        case TextureType::Texture3D:
            BuildTexture3D(textureDesc, imageDesc);
            break;
        case TextureType::TextureCube:
            BuildTextureCube(textureDesc, imageDesc);
            break;
        case TextureType::Texture1DArray:
            BuildTexture1DArray(textureDesc, imageDesc);
            break;
        case TextureType::Texture2DArray:
            BuildTexture2DArray(textureDesc, imageDesc);
            break;
        case TextureType::TextureCubeArray:
            BuildTextureCubeArray(textureDesc, imageDesc);
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
    glGetTexLevelParameteriv(target, 0, GL_TEXTURE_WIDTH, &desc.texture3DDesc.width);
    glGetTexLevelParameteriv(target, 0, GL_TEXTURE_HEIGHT, &desc.texture3DDesc.height);
    glGetTexLevelParameteriv(target, 0, GL_TEXTURE_DEPTH, &desc.texture3DDesc.depth);

    if (desc.type == TextureType::TextureCube || desc.type == TextureType::TextureCubeArray)
        desc.texture3DDesc.depth /= 6;

    return desc;
}

/* ----- "BuildTexture..." functions ----- */

void GLRenderSystem::BindTextureAndSetType(GLTexture& textureGL, const TextureType type)
{
    if (textureGL.GetType() == type)
    {
        /* If type is already set, just bind texture */
        GLStateManager::active->BindTexture(textureGL);
    }
    else
    {
        /* If texture is not undefined -> recreate it */
        if (textureGL.GetType() != TextureType::Undefined)
            textureGL.Recreate();

        /* Set type of the specified texture for the first time */
        textureGL.SetType(type);
        GLStateManager::active->ForcedBindTexture(textureGL);

        /* Setup texture parameters for the first time */
        auto target = GLTypes::Map(type);
        glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
}

static void GLTexImage1DBase(
    GLenum target, const TextureFormat internalFormat, int width,
    GLenum format, GLenum type, const void* data, unsigned int compressedSize)
{
    if (IsCompressedFormat(internalFormat))
        glCompressedTexImage1D(target, 0, GLTypes::Map(internalFormat), width, 0, static_cast<GLsizei>(compressedSize), data);
    else
        glTexImage1D(target, 0, GLTypes::Map(internalFormat), width, 0, format, type, data);
}

static void GLTexImage2DBase(
    GLenum target, const TextureFormat internalFormat, int width, int height,
    GLenum format, GLenum type, const void* data, unsigned int compressedSize)
{
    if (IsCompressedFormat(internalFormat))
        glCompressedTexImage2D(target, 0, GLTypes::Map(internalFormat), width, height, 0, static_cast<GLsizei>(compressedSize), data);
    else
        glTexImage2D(target, 0, GLTypes::Map(internalFormat), width, height, 0, format, type, data);
}

static void GLTexImage3DBase(
    GLenum target, const TextureFormat internalFormat, int width, int height, int depth,
    GLenum format, GLenum type, const void* data, unsigned int compressedSize)
{
    if (IsCompressedFormat(internalFormat))
        glCompressedTexImage3D(target, 0, GLTypes::Map(internalFormat), width, height, depth, 0, static_cast<GLsizei>(compressedSize), data);
    else
        glTexImage3D(target, 0, GLTypes::Map(internalFormat), width, height, depth, 0, format, type, data);
}

static void GLTexImage1D(
    const TextureFormat internalFormat, int width,
    GLenum format, GLenum type, const void* data, unsigned int compressedSize = 0)
{
    GLTexImage1DBase(
        GL_TEXTURE_1D,
        internalFormat, width,
        format, type, data, compressedSize
    );
}

static void GLTexImage2D(
    const TextureFormat internalFormat, int width, int height,
    GLenum format, GLenum type, const void* data, unsigned int compressedSize = 0)
{
    GLTexImage2DBase(
        GL_TEXTURE_2D,
        internalFormat, width, height,
        format, type, data, compressedSize
    );
}

static void GLTexImage3D(
    const TextureFormat internalFormat, int width, int height, int depth,
    GLenum format, GLenum type, const void* data, unsigned int compressedSize = 0)
{
    GLTexImage3DBase(
        GL_TEXTURE_3D,
        internalFormat, width, height, depth,
        format, type, data, compressedSize
    );
}

static void GLTexImageCube(
    const TextureFormat internalFormat, int width, int height, AxisDirection cubeFace,
    GLenum format, GLenum type, const void* data, unsigned int compressedSize = 0)
{
    GLTexImage2DBase(
        GLTypes::Map(cubeFace),
        internalFormat, width, height,
        format, type, data, compressedSize
    );
}

static void GLTexImage1DArray(
    const TextureFormat internalFormat, int width, unsigned int layers,
    GLenum format, GLenum type, const void* data, unsigned int compressedSize = 0)
{
    GLTexImage2DBase(
        GL_TEXTURE_1D_ARRAY,
        internalFormat, width, static_cast<GLsizei>(layers),
        format, type, data, compressedSize
    );
}

static void GLTexImage2DArray(
    const TextureFormat internalFormat, int width, int height, unsigned int layers,
    GLenum format, GLenum type, const void* data, unsigned int compressedSize = 0)
{
    GLTexImage3DBase(
        GL_TEXTURE_2D_ARRAY,
        internalFormat, width, height, static_cast<GLsizei>(layers),
        format, type, data, compressedSize);
}

static void GLTexImageCubeArray(
    const TextureFormat internalFormat, int width, int height, unsigned int layers,
    GLenum format, GLenum type, const void* data, unsigned int compressedSize = 0)
{
    GLTexImage3DBase(
        GL_TEXTURE_CUBE_MAP_ARRAY,
        internalFormat, width, height, static_cast<GLsizei>(layers)*6,
        format, type, data, compressedSize
    );
}

void GLRenderSystem::BuildTexture1D(const TextureDescriptor& desc, const ImageDescriptor* imageDesc)
{
    if (imageDesc)
    {
        /* Setup texture image from descriptor */
        GLTexImage1D(
            desc.format, desc.texture1DDesc.width,
            GLTypes::Map(imageDesc->format), GLTypes::Map(imageDesc->dataType), imageDesc->buffer, imageDesc->compressedSize
        );
    }
    else if (IsCompressedFormat(desc.format))
    {
        /* Initialize compressed texture image with null pointer */
        GLTexImage1D(desc.format, desc.texture1DDesc.width, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    }
    else
    {
        /* Initialize texture image with default color */
        auto image = GetDefaultTextureImageRGBAub(desc.texture1DDesc.width);
        GLTexImage1D(desc.format, desc.texture1DDesc.width, GL_RGBA, GL_UNSIGNED_BYTE, image.data());
    }
}

void GLRenderSystem::BuildTexture2D(const TextureDescriptor& desc, const ImageDescriptor* imageDesc)
{
    if (imageDesc)
    {
        /* Setup texture image from descriptor */
        GLTexImage2D(
            desc.format, desc.texture2DDesc.width, desc.texture2DDesc.height,
            GLTypes::Map(imageDesc->format), GLTypes::Map(imageDesc->dataType), imageDesc->buffer, imageDesc->compressedSize
        );
    }
    else if (IsCompressedFormat(desc.format))
    {
        /* Initialize compressed texture image with null pointer */
        GLTexImage2D(desc.format, desc.texture2DDesc.width, desc.texture2DDesc.height, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    }
    else
    {
        /* Initialize texture image with default color */
        auto image = GetDefaultTextureImageRGBAub(desc.texture2DDesc.width * desc.texture2DDesc.height);
        GLTexImage2D(desc.format, desc.texture2DDesc.width, desc.texture2DDesc.height, GL_RGBA, GL_UNSIGNED_BYTE, image.data());
    }
}

void GLRenderSystem::BuildTexture3D(const TextureDescriptor& desc, const ImageDescriptor* imageDesc)
{
    LLGL_ASSERT_CAP(has3DTextures);

    if (imageDesc)
    {
        /* Setup texture image from descriptor */
        GLTexImage3D(
            desc.format, desc.texture3DDesc.width, desc.texture3DDesc.height, desc.texture3DDesc.depth,
            GLTypes::Map(imageDesc->format), GLTypes::Map(imageDesc->dataType), imageDesc->buffer, imageDesc->compressedSize
        );
    }
    else if (IsCompressedFormat(desc.format))
    {
        /* Initialize compressed texture image with null pointer */
        GLTexImage3D(desc.format, desc.texture3DDesc.width, desc.texture3DDesc.height, desc.texture3DDesc.depth, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    }
    else
    {
        /* Initialize texture image with default color */
        auto image = GetDefaultTextureImageRGBAub(desc.texture3DDesc.width * desc.texture3DDesc.height * desc.texture3DDesc.depth);
        GLTexImage3D(desc.format, desc.texture3DDesc.width, desc.texture3DDesc.height, desc.texture3DDesc.depth, GL_RGBA, GL_UNSIGNED_BYTE, image.data());
    }
}

void GLRenderSystem::BuildTextureCube(const TextureDescriptor& desc, const ImageDescriptor* imageDesc)
{
    LLGL_ASSERT_CAP(hasCubeTextures);

    const std::array<AxisDirection, 6> cubeFaces
    {{
        AxisDirection::XPos,
        AxisDirection::XNeg,
        AxisDirection::YPos,
        AxisDirection::YNeg,
        AxisDirection::ZPos,
        AxisDirection::ZNeg
    }};

    if (imageDesc)
    {
        /* Setup texture image cube-faces from descriptor */
        auto imageFace          = reinterpret_cast<const char*>(imageDesc->buffer);
        auto imageFaceStride    = (desc.textureCubeDesc.width * desc.textureCubeDesc.height * ImageFormatSize(imageDesc->format) * DataTypeSize(imageDesc->dataType));

        if (IsCompressedFormat(desc.format))
            imageFaceStride = imageDesc->compressedSize;

        auto dataFormatGL       = GLTypes::Map(imageDesc->format);
        auto dataTypeGL         = GLTypes::Map(imageDesc->dataType);

        for (auto face : cubeFaces)
        {
            GLTexImageCube(
                desc.format, desc.textureCubeDesc.width, desc.textureCubeDesc.height,
                face, dataFormatGL, dataTypeGL, imageFace, imageDesc->compressedSize
            );
            imageFace += imageFaceStride;
        }
    }
    else if (IsCompressedFormat(desc.format))
    {
        /* Initialize compressed texture image with null pointer */
        for (auto face : cubeFaces)
            GLTexImageCube(desc.format, desc.textureCubeDesc.width, desc.textureCubeDesc.height, face, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    }
    else
    {
        /* Initialize texture image cube-faces with default color */
        auto image = GetDefaultTextureImageRGBAub(desc.textureCubeDesc.width * desc.textureCubeDesc.height);
        for (auto face : cubeFaces)
            GLTexImageCube(desc.format, desc.textureCubeDesc.width, desc.textureCubeDesc.height, face, GL_RGBA, GL_UNSIGNED_BYTE, image.data());
    }
}

void GLRenderSystem::BuildTexture1DArray(const TextureDescriptor& desc, const ImageDescriptor* imageDesc)
{
    LLGL_ASSERT_CAP(hasTextureArrays);

    if (imageDesc)
    {
        /* Setup texture image from descriptor */
        GLTexImage1DArray(
            desc.format, desc.texture1DDesc.width, desc.texture1DDesc.layers,
            GLTypes::Map(imageDesc->format), GLTypes::Map(imageDesc->dataType), imageDesc->buffer, imageDesc->compressedSize
        );
    }
    else if (IsCompressedFormat(desc.format))
    {
        /* Initialize compressed texture image with null pointer */
        GLTexImage1DArray(desc.format, desc.texture1DDesc.width, desc.texture1DDesc.layers, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    }
    else
    {
        /* Initialize texture image with default color */
        auto image = GetDefaultTextureImageRGBAub(desc.texture1DDesc.width * static_cast<int>(desc.texture1DDesc.layers));
        GLTexImage1DArray(desc.format, desc.texture1DDesc.width, desc.texture1DDesc.layers, GL_RGBA, GL_UNSIGNED_BYTE, image.data());
    }
}

void GLRenderSystem::BuildTexture2DArray(const TextureDescriptor& desc, const ImageDescriptor* imageDesc)
{
    LLGL_ASSERT_CAP(hasTextureArrays);

    if (imageDesc)
    {
        /* Setup texture image from descriptor */
        GLTexImage2DArray(
            desc.format, desc.texture2DDesc.width, desc.texture2DDesc.height, desc.texture2DDesc.layers,
            GLTypes::Map(imageDesc->format), GLTypes::Map(imageDesc->dataType), imageDesc->buffer, imageDesc->compressedSize
        );
    }
    else if (IsCompressedFormat(desc.format))
    {
        /* Initialize compressed texture image with null pointer */
        GLTexImage2DArray(desc.format, desc.texture2DDesc.width, desc.texture2DDesc.height, desc.texture2DDesc.layers, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    }
    else
    {
        /* Initialize texture image with default color */
        auto image = GetDefaultTextureImageRGBAub(desc.texture2DDesc.width * desc.texture2DDesc.height * static_cast<int>(desc.texture2DDesc.layers));
        GLTexImage2DArray(desc.format, desc.texture2DDesc.width, desc.texture2DDesc.height, desc.texture2DDesc.layers, GL_RGBA, GL_UNSIGNED_BYTE, image.data());
    }
}

void GLRenderSystem::BuildTextureCubeArray(const TextureDescriptor& desc, const ImageDescriptor* imageDesc)
{
    LLGL_ASSERT_CAP(hasCubeTextureArrays);

    if (imageDesc)
    {
        /* Setup texture image cube-faces from descriptor */
        GLTexImageCubeArray(
            desc.format, desc.textureCubeDesc.width, desc.textureCubeDesc.height, desc.textureCubeDesc.layers,
            GLTypes::Map(imageDesc->format), GLTypes::Map(imageDesc->dataType), imageDesc->buffer, imageDesc->compressedSize
        );
    }
    else if (IsCompressedFormat(desc.format))
    {
        /* Initialize compressed texture image with null pointer */
        GLTexImageCubeArray(desc.format, desc.textureCubeDesc.width, desc.textureCubeDesc.height, desc.textureCubeDesc.layers, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    }
    else
    {
        /* Initialize texture image cube-faces with default color */
        auto image = GetDefaultTextureImageRGBAub(desc.textureCubeDesc.width * desc.textureCubeDesc.height * static_cast<int>(desc.textureCubeDesc.layers*6));
        GLTexImageCubeArray(desc.format, desc.textureCubeDesc.width, desc.textureCubeDesc.height, desc.textureCubeDesc.layers, GL_RGBA, GL_UNSIGNED_BYTE, image.data());
    }
}

/* ----- "WriteTexture..." functions ----- */

static void GLTexSubImage1DBase(GLenum target, int mipLevel, int x, int width, const ImageDescriptor& imageDesc)
{
    if (IsCompressedFormat(imageDesc.format))
    {
        glCompressedTexSubImage1D(
            target, mipLevel, x, width,
            GLTypes::Map(imageDesc.format), static_cast<GLsizei>(imageDesc.compressedSize), imageDesc.buffer
        );
    }
    else
    {
        glTexSubImage1D(
            target, mipLevel, x, width,
            GLTypes::Map(imageDesc.format), GLTypes::Map(imageDesc.dataType), imageDesc.buffer
        );
    }
}

static void GLTexSubImage2DBase(GLenum target, int mipLevel, int x, int y, int width, int height, const ImageDescriptor& imageDesc)
{
    if (IsCompressedFormat(imageDesc.format))
    {
        glCompressedTexSubImage2D(
            target, mipLevel, x, y, width, height,
            GLTypes::Map(imageDesc.format), static_cast<GLsizei>(imageDesc.compressedSize), imageDesc.buffer
        );
    }
    else
    {
        glTexSubImage2D(
            target, mipLevel, x, y, width, height,
            GLTypes::Map(imageDesc.format), GLTypes::Map(imageDesc.dataType), imageDesc.buffer
        );
    }
}

static void GLTexSubImage3DBase(GLenum target, int mipLevel, int x, int y, int z, int width, int height, int depth, const ImageDescriptor& imageDesc)
{
    if (IsCompressedFormat(imageDesc.format))
    {
        glCompressedTexSubImage3D(
            target, mipLevel, x, y, z, width, height, depth,
            GLTypes::Map(imageDesc.format), static_cast<GLsizei>(imageDesc.compressedSize), imageDesc.buffer
        );
    }
    else
    {
        glTexSubImage3D(
            target, mipLevel, x, y, z, width, height, depth,
            GLTypes::Map(imageDesc.format), GLTypes::Map(imageDesc.dataType), imageDesc.buffer
        );
    }
}

static void GLTexSubImage1D(int mipLevel, int x, int width, const ImageDescriptor& imageDesc)
{
    GLTexSubImage1DBase(GL_TEXTURE_1D, mipLevel, x, width, imageDesc);
}

static void GLTexSubImage2D(int mipLevel, int x, int y, int width, int height, const ImageDescriptor& imageDesc)
{
    GLTexSubImage2DBase(GL_TEXTURE_2D, mipLevel, x, y, width, height, imageDesc);
}

static void GLTexSubImage3D(int mipLevel, int x, int y, int z, int width, int height, int depth, const ImageDescriptor& imageDesc)
{
    GLTexSubImage3DBase(GL_TEXTURE_3D, mipLevel, x, y, z, width, height, depth, imageDesc);
}

static void GLTexSubImageCube(int mipLevel, int x, int y, int width, int height, AxisDirection cubeFace, const ImageDescriptor& imageDesc)
{
    GLTexSubImage2DBase(GLTypes::Map(cubeFace), mipLevel, x, y, width, height, imageDesc);
}

static void GLTexSubImage1DArray(int mipLevel, int x, unsigned int layerOffset, int width, unsigned int layers, const ImageDescriptor& imageDesc)
{
    GLTexSubImage2DBase(
        GL_TEXTURE_1D_ARRAY,
        mipLevel, x, static_cast<GLsizei>(layerOffset),
        width, static_cast<GLsizei>(layers), imageDesc
    );
}

static void GLTexSubImage2DArray(
    int mipLevel, int x, int y, unsigned int layerOffset, int width, int height, unsigned int layers, const ImageDescriptor& imageDesc)
{
    GLTexSubImage3DBase(
        GL_TEXTURE_2D_ARRAY,
        mipLevel, x, y, static_cast<GLsizei>(layerOffset),
        width, height, static_cast<GLsizei>(layers), imageDesc
    );
}

static void GLTexSubImageCubeArray(
    int mipLevel, int x, int y, unsigned int layerOffset, AxisDirection cubeFaceOffset,
    int width, int height, unsigned int cubeFaces, const ImageDescriptor& imageDesc)
{
    GLTexSubImage3DBase(
        GL_TEXTURE_CUBE_MAP_ARRAY,
        mipLevel, x, y, static_cast<GLsizei>(layerOffset + static_cast<int>(cubeFaceOffset))*6,
        width, height, static_cast<GLsizei>(cubeFaces), imageDesc
    );
}

void GLRenderSystem::WriteTexture1D(
    Texture& texture, int mipLevel, int position, int size, const ImageDescriptor& imageDesc)
{
    /* Bind texture and write texture sub data */
    auto& textureGL = LLGL_CAST(GLTexture&, texture);
    GLStateManager::active->BindTexture(textureGL);

    GLTexSubImage1D(mipLevel, position, size, imageDesc);
}

void GLRenderSystem::WriteTexture2D(
    Texture& texture, int mipLevel, const Gs::Vector2i& position, const Gs::Vector2i& size, const ImageDescriptor& imageDesc)
{
    /* Bind texture and write texture sub data */
    auto& textureGL = LLGL_CAST(GLTexture&, texture);
    GLStateManager::active->BindTexture(textureGL);

    GLTexSubImage2D(mipLevel, position.x, position.y, size.x, size.y, imageDesc);
}

void GLRenderSystem::WriteTexture3D(
    Texture& texture, int mipLevel, const Gs::Vector3i& position, const Gs::Vector3i& size, const ImageDescriptor& imageDesc)
{
    LLGL_ASSERT_CAP(has3DTextures);

    /* Bind texture and write texture sub data */
    auto& textureGL = LLGL_CAST(GLTexture&, texture);
    GLStateManager::active->BindTexture(textureGL);

    GLTexSubImage3D(mipLevel, position.x, position.y, position.z, size.x, size.y, size.z, imageDesc);
}

void GLRenderSystem::WriteTextureCube(
    Texture& texture, int mipLevel, const Gs::Vector2i& position, const AxisDirection cubeFace, const Gs::Vector2i& size, const ImageDescriptor& imageDesc)
{
    LLGL_ASSERT_CAP(hasCubeTextures);

    /* Bind texture and write texture sub data */
    auto& textureGL = LLGL_CAST(GLTexture&, texture);
    GLStateManager::active->BindTexture(textureGL);

    GLTexSubImageCube(mipLevel, position.x, position.y, size.x, size.y, cubeFace, imageDesc);
}

void GLRenderSystem::WriteTexture1DArray(
    Texture& texture, int mipLevel, int position, unsigned int layerOffset,
    int size, unsigned int layers, const ImageDescriptor& imageDesc)
{
    LLGL_ASSERT_CAP(hasTextureArrays);

    /* Bind texture and write texture sub data */
    auto& textureGL = LLGL_CAST(GLTexture&, texture);
    GLStateManager::active->BindTexture(textureGL);

    GLTexSubImage1DArray(mipLevel, position, layerOffset, size, layers, imageDesc);
}

void GLRenderSystem::WriteTexture2DArray(
    Texture& texture, int mipLevel, const Gs::Vector2i& position, unsigned int layerOffset,
    const Gs::Vector2i& size, unsigned int layers, const ImageDescriptor& imageDesc)
{
    LLGL_ASSERT_CAP(hasTextureArrays);

    /* Bind texture and write texture sub data */
    auto& textureGL = LLGL_CAST(GLTexture&, texture);
    GLStateManager::active->BindTexture(textureGL);

    GLTexSubImage2DArray(mipLevel, position.x, position.y, layerOffset, size.x, size.y, layers, imageDesc);
}

void GLRenderSystem::WriteTextureCubeArray(
    Texture& texture, int mipLevel, const Gs::Vector2i& position, unsigned int layerOffset, const AxisDirection cubeFaceOffset,
    const Gs::Vector2i& size, unsigned int cubeFaces, const ImageDescriptor& imageDesc)
{
    LLGL_ASSERT_CAP(hasCubeTextureArrays);

    /* Bind texture and write texture sub data */
    auto& textureGL = LLGL_CAST(GLTexture&, texture);
    GLStateManager::active->BindTexture(textureGL);

    GLTexSubImageCubeArray(mipLevel, position.x, position.y, layerOffset, cubeFaceOffset, size.x, size.y, cubeFaces, imageDesc);
}

void GLRenderSystem::ReadTexture(const Texture& texture, int mipLevel, ImageFormat dataFormat, DataType dataType, void* data)
{
    LLGL_ASSERT_PTR(data);

    /* Bind texture */
    auto& textureGL = LLGL_CAST(const GLTexture&, texture);
    GLStateManager::active->BindTexture(textureGL);

    /* Read image data from texture */
    glGetTexImage(
        GLTypes::Map(textureGL.GetType()),
        mipLevel,
        GLTypes::Map(dataFormat),
        GLTypes::Map(dataType),
        data
    );
}


} // /namespace LLGL



// ================================================================================
