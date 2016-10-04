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
    GLint texSize[3] = { 0 };
    glGetTexLevelParameteriv(target, 0, GL_TEXTURE_WIDTH,  &texSize[0]);
    glGetTexLevelParameteriv(target, 0, GL_TEXTURE_HEIGHT, &texSize[1]);
    glGetTexLevelParameteriv(target, 0, GL_TEXTURE_DEPTH,  &texSize[2]);

    desc.texture3DDesc.width    = static_cast<unsigned int>(texSize[0]);
    desc.texture3DDesc.height   = static_cast<unsigned int>(texSize[1]);
    desc.texture3DDesc.depth    = static_cast<unsigned int>(texSize[2]);

    if (desc.type == TextureType::TextureCube || desc.type == TextureType::TextureCubeArray)
        desc.texture3DDesc.depth /= 6;

    return desc;
}

/* ----- "BuildTexture..." functions ----- */

static void GLTexImage1DBase(
    GLenum target, const TextureFormat internalFormat, unsigned int width,
    GLenum format, GLenum type, const void* data, unsigned int compressedSize)
{
    if (IsCompressedFormat(internalFormat))
    {
        glCompressedTexImage1D(
            target, 0, GLTypes::Map(internalFormat), static_cast<GLsizei>(width), 0, static_cast<GLsizei>(compressedSize), data
        );
    }
    else
    {
        glTexImage1D(
            target, 0, GLTypes::Map(internalFormat), static_cast<GLsizei>(width), 0, format, type, data
        );
    }
}

static void GLTexImage2DBase(
    GLenum target, const TextureFormat internalFormat, unsigned int width, unsigned int height,
    GLenum format, GLenum type, const void* data, unsigned int compressedSize)
{
    if (IsCompressedFormat(internalFormat))
    {
        glCompressedTexImage2D(
            target, 0, GLTypes::Map(internalFormat),
            static_cast<GLsizei>(width), static_cast<GLsizei>(height),
            0, static_cast<GLsizei>(compressedSize), data
        );
    }
    else
    {
        glTexImage2D(
            target, 0, GLTypes::Map(internalFormat),
            static_cast<GLsizei>(width), static_cast<GLsizei>(height),
            0, format, type, data
        );
    }
}

static void GLTexImage3DBase(
    GLenum target, const TextureFormat internalFormat, unsigned int width, unsigned int height, unsigned int depth,
    GLenum format, GLenum type, const void* data, unsigned int compressedSize)
{
    if (IsCompressedFormat(internalFormat))
    {
        glCompressedTexImage3D(
            target, 0, GLTypes::Map(internalFormat),
            static_cast<GLsizei>(width), static_cast<GLsizei>(height), static_cast<GLsizei>(depth),
            0, static_cast<GLsizei>(compressedSize), data
        );
    }
    else
    {
        glTexImage3D(
            target, 0, GLTypes::Map(internalFormat),
            static_cast<GLsizei>(width), static_cast<GLsizei>(height), static_cast<GLsizei>(depth),
            0, format, type, data
        );
    }
}

static void GLTexImage1D(
    const TextureFormat internalFormat, unsigned int width,
    GLenum format, GLenum type, const void* data, unsigned int compressedSize = 0)
{
    GLTexImage1DBase(
        GL_TEXTURE_1D,
        internalFormat, width,
        format, type, data, compressedSize
    );
}

static void GLTexImage2D(
    const TextureFormat internalFormat, unsigned int width, unsigned int height,
    GLenum format, GLenum type, const void* data, unsigned int compressedSize = 0)
{
    GLTexImage2DBase(
        GL_TEXTURE_2D,
        internalFormat, width, height,
        format, type, data, compressedSize
    );
}

static void GLTexImage3D(
    const TextureFormat internalFormat, unsigned int width, unsigned int height, unsigned int depth,
    GLenum format, GLenum type, const void* data, unsigned int compressedSize = 0)
{
    GLTexImage3DBase(
        GL_TEXTURE_3D,
        internalFormat, width, height, depth,
        format, type, data, compressedSize
    );
}

static void GLTexImageCube(
    const TextureFormat internalFormat, unsigned int width, unsigned int height, AxisDirection cubeFace,
    GLenum format, GLenum type, const void* data, unsigned int compressedSize = 0)
{
    GLTexImage2DBase(
        GLTypes::Map(cubeFace),
        internalFormat, width, height,
        format, type, data, compressedSize
    );
}

static void GLTexImage1DArray(
    const TextureFormat internalFormat, unsigned int width, unsigned int layers,
    GLenum format, GLenum type, const void* data, unsigned int compressedSize = 0)
{
    GLTexImage2DBase(
        GL_TEXTURE_1D_ARRAY,
        internalFormat, width, static_cast<GLsizei>(layers),
        format, type, data, compressedSize
    );
}

static void GLTexImage2DArray(
    const TextureFormat internalFormat, unsigned int width, unsigned int height, unsigned int layers,
    GLenum format, GLenum type, const void* data, unsigned int compressedSize = 0)
{
    GLTexImage3DBase(
        GL_TEXTURE_2D_ARRAY,
        internalFormat, width, height, static_cast<GLsizei>(layers),
        format, type, data, compressedSize
    );
}

static void GLTexImageCubeArray(
    const TextureFormat internalFormat, unsigned int width, unsigned int height, unsigned int layers,
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

void GLRenderSystem::WriteTexture(Texture& texture, const SubTextureDescriptor& subTextureDesc, const ImageDescriptor& imageDesc)
{
    /* Bind texture and write texture sub data */
    auto& textureGL = LLGL_CAST(GLTexture&, texture);
    GLStateManager::active->BindTexture(textureGL);

    /* Write data into specific texture type */
    switch (texture.GetType())
    {
        case TextureType::Texture1D:
            WriteTexture1D(subTextureDesc, imageDesc);
            break;
        case TextureType::Texture2D:
            WriteTexture2D(subTextureDesc, imageDesc);
            break;
        case TextureType::Texture3D:
            WriteTexture3D(subTextureDesc, imageDesc);
            break;
        case TextureType::TextureCube:
            WriteTextureCube(subTextureDesc, imageDesc);
            break;
        case TextureType::Texture1DArray:
            WriteTexture1DArray(subTextureDesc, imageDesc);
            break;
        case TextureType::Texture2DArray:
            WriteTexture2DArray(subTextureDesc, imageDesc);
            break;
        case TextureType::TextureCubeArray:
            WriteTextureCubeArray(subTextureDesc, imageDesc);
            break;
    }
}

static void GLTexSubImage1DBase(
    GLenum target, unsigned int mipLevel, unsigned int x, unsigned int width, const ImageDescriptor& imageDesc)
{
    if (IsCompressedFormat(imageDesc.format))
    {
        glCompressedTexSubImage1D(
            target,
            static_cast<GLint>(mipLevel),
            static_cast<GLint>(x),
            static_cast<GLsizei>(width),
            GLTypes::Map(imageDesc.format),
            static_cast<GLsizei>(imageDesc.compressedSize),
            imageDesc.buffer
        );
    }
    else
    {
        glTexSubImage1D(
            target,
            static_cast<GLint>(mipLevel),
            static_cast<GLint>(x),
            static_cast<GLsizei>(width),
            GLTypes::Map(imageDesc.format),
            GLTypes::Map(imageDesc.dataType),
            imageDesc.buffer
        );
    }
}

static void GLTexSubImage2DBase(
    GLenum target, unsigned int mipLevel, unsigned int x, unsigned int y,
    unsigned int width, unsigned int height, const ImageDescriptor& imageDesc)
{
    if (IsCompressedFormat(imageDesc.format))
    {
        glCompressedTexSubImage2D(
            target,
            static_cast<GLint>(mipLevel),
            static_cast<GLint>(x),
            static_cast<GLint>(y),
            static_cast<GLsizei>(width),
            static_cast<GLsizei>(height),
            GLTypes::Map(imageDesc.format),
            static_cast<GLsizei>(imageDesc.compressedSize), imageDesc.buffer
        );
    }
    else
    {
        glTexSubImage2D(
            target,
            static_cast<GLint>(mipLevel),
            static_cast<GLint>(x),
            static_cast<GLint>(y),
            static_cast<GLsizei>(width),
            static_cast<GLsizei>(height),
            GLTypes::Map(imageDesc.format),
            GLTypes::Map(imageDesc.dataType),
            imageDesc.buffer
        );
    }
}

static void GLTexSubImage3DBase(
    GLenum target, unsigned int mipLevel, unsigned int x, unsigned int y, unsigned int z,
    unsigned int width, unsigned int height, unsigned int depth, const ImageDescriptor& imageDesc)
{
    if (IsCompressedFormat(imageDesc.format))
    {
        glCompressedTexSubImage3D(
            target,
            static_cast<GLint>(mipLevel),
            static_cast<GLint>(x),
            static_cast<GLint>(y),
            static_cast<GLint>(z),
            static_cast<GLsizei>(width),
            static_cast<GLsizei>(height),
            static_cast<GLsizei>(depth),
            GLTypes::Map(imageDesc.format),
            static_cast<GLsizei>(imageDesc.compressedSize),
            imageDesc.buffer
        );
    }
    else
    {
        glTexSubImage3D(
            target,
            static_cast<GLint>(mipLevel),
            static_cast<GLint>(x),
            static_cast<GLint>(y),
            static_cast<GLint>(z),
            static_cast<GLsizei>(width),
            static_cast<GLsizei>(height),
            static_cast<GLsizei>(depth),
            GLTypes::Map(imageDesc.format),
            GLTypes::Map(imageDesc.dataType),
            imageDesc.buffer
        );
    }
}

static void GLTexSubImage1D(
    unsigned int mipLevel, unsigned int x, unsigned int width, const ImageDescriptor& imageDesc)
{
    GLTexSubImage1DBase(GL_TEXTURE_1D, mipLevel, x, width, imageDesc);
}

static void GLTexSubImage2D(
    unsigned int mipLevel, unsigned int x, unsigned int y,
    unsigned int width, unsigned int height, const ImageDescriptor& imageDesc)
{
    GLTexSubImage2DBase(GL_TEXTURE_2D, mipLevel, x, y, width, height, imageDesc);
}

static void GLTexSubImage3D(
    unsigned int mipLevel, unsigned int x, unsigned int y, unsigned int z,
    unsigned int width, unsigned int height, unsigned int depth, const ImageDescriptor& imageDesc)
{
    GLTexSubImage3DBase(GL_TEXTURE_3D, mipLevel, x, y, z, width, height, depth, imageDesc);
}

static void GLTexSubImageCube(
    unsigned int mipLevel, unsigned int x, unsigned int y,
    unsigned int width, unsigned int height, AxisDirection cubeFace, const ImageDescriptor& imageDesc)
{
    GLTexSubImage2DBase(GLTypes::Map(cubeFace), mipLevel, x, y, width, height, imageDesc);
}

static void GLTexSubImage1DArray(
    unsigned int mipLevel, unsigned int x, unsigned int layerOffset,
    unsigned int width, unsigned int layers, const ImageDescriptor& imageDesc)
{
    GLTexSubImage2DBase(GL_TEXTURE_1D_ARRAY, mipLevel, x, layerOffset, width, layers, imageDesc);
}

static void GLTexSubImage2DArray(
    int mipLevel, int x, int y, unsigned int layerOffset,
    int width, int height, unsigned int layers, const ImageDescriptor& imageDesc)
{
    GLTexSubImage3DBase(GL_TEXTURE_2D_ARRAY, mipLevel, x, y, layerOffset, width, height, layers, imageDesc);
}

static void GLTexSubImageCubeArray(
    unsigned int mipLevel, unsigned int x, unsigned int y, unsigned int layerOffset, AxisDirection cubeFaceOffset,
    unsigned int width, unsigned int height, unsigned int cubeFaces, const ImageDescriptor& imageDesc)
{
    layerOffset = layerOffset * 6 + static_cast<unsigned int>(cubeFaceOffset);
    GLTexSubImage3DBase(GL_TEXTURE_CUBE_MAP_ARRAY, mipLevel, x, y, layerOffset, width, height, cubeFaces, imageDesc);
}

void GLRenderSystem::WriteTexture1D(const SubTextureDescriptor& desc, const ImageDescriptor& imageDesc)
{
    GLTexSubImage1D(desc.mipLevel, desc.texture1DDesc.x, desc.texture1DDesc.width, imageDesc);
}

void GLRenderSystem::WriteTexture2D(const SubTextureDescriptor& desc, const ImageDescriptor& imageDesc)
{
    GLTexSubImage2D(
        desc.mipLevel, desc.texture2DDesc.x, desc.texture2DDesc.y,
        desc.texture2DDesc.width, desc.texture2DDesc.height, imageDesc
    );
}

void GLRenderSystem::WriteTexture3D(const SubTextureDescriptor& desc, const ImageDescriptor& imageDesc)
{
    LLGL_ASSERT_CAP(has3DTextures);
    GLTexSubImage3D(
        desc.mipLevel, desc.texture3DDesc.x, desc.texture3DDesc.y, desc.texture3DDesc.z,
        desc.texture3DDesc.width, desc.texture3DDesc.height, desc.texture3DDesc.depth, imageDesc
    );
}

void GLRenderSystem::WriteTextureCube(const SubTextureDescriptor& desc, const ImageDescriptor& imageDesc)
{
    LLGL_ASSERT_CAP(hasCubeTextures);
    GLTexSubImageCube(
        desc.mipLevel, desc.textureCubeDesc.x, desc.textureCubeDesc.y,
        desc.textureCubeDesc.width, desc.textureCubeDesc.height, desc.textureCubeDesc.cubeFaceOffset, imageDesc
    );
}

void GLRenderSystem::WriteTexture1DArray(const SubTextureDescriptor& desc, const ImageDescriptor& imageDesc)
{
    LLGL_ASSERT_CAP(hasTextureArrays);
    GLTexSubImage1DArray(
        desc.mipLevel, desc.texture1DDesc.x, desc.texture1DDesc.layerOffset,
        desc.texture1DDesc.width, desc.texture1DDesc.layers, imageDesc
    );
}

void GLRenderSystem::WriteTexture2DArray(const SubTextureDescriptor& desc, const ImageDescriptor& imageDesc)
{
    LLGL_ASSERT_CAP(hasTextureArrays);
    GLTexSubImage2DArray(
        desc.mipLevel, desc.texture2DDesc.x, desc.texture2DDesc.y, desc.texture2DDesc.layerOffset,
        desc.texture2DDesc.width, desc.texture2DDesc.height, desc.texture2DDesc.layers, imageDesc
    );
}

void GLRenderSystem::WriteTextureCubeArray(const SubTextureDescriptor& desc, const ImageDescriptor& imageDesc)
{
    LLGL_ASSERT_CAP(hasCubeTextureArrays);
    GLTexSubImageCubeArray(
        desc.mipLevel, desc.textureCubeDesc.x, desc.textureCubeDesc.y, desc.textureCubeDesc.layerOffset, desc.textureCubeDesc.cubeFaceOffset,
        desc.textureCubeDesc.width, desc.textureCubeDesc.height, desc.textureCubeDesc.cubeFaces, imageDesc
    );
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
