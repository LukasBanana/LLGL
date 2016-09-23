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

Texture* GLRenderSystem::CreateTexture()
{
    return TakeOwnership(textures_, MakeUnique<GLTexture>());
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
    InitMemory(desc);

    desc.type = texture.GetType();

    /* Query texture size */
    auto target = GLTypes::Map(texture.GetType());

    glGetTexLevelParameteriv(target, 0, GL_TEXTURE_WIDTH, &desc.texture3DDesc.width);
    glGetTexLevelParameteriv(target, 0, GL_TEXTURE_HEIGHT, &desc.texture3DDesc.height);
    glGetTexLevelParameteriv(target, 0, GL_TEXTURE_DEPTH, &desc.texture3DDesc.depth);

    return desc;
}

/* ----- "SetupTexture..." functions ----- */

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

void GLRenderSystem::SetupTexture1D(Texture& texture, const TextureFormat format, int size, const ImageDescriptor* imageDesc)
{
    /* Bind texture and set type */
    auto& textureGL = LLGL_CAST(GLTexture&, texture);
    BindTextureAndSetType(textureGL, TextureType::Texture1D);

    if (imageDesc)
    {
        /* Setup texture image from descriptor */
        GLTexImage1D(format, size, GLTypes::Map(imageDesc->format), GLTypes::Map(imageDesc->dataType), imageDesc->buffer, imageDesc->compressedSize);
    }
    else if (IsCompressedFormat(format))
    {
        /* Initialize compressed texture image with null pointer */
        GLTexImage1D(format, size, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    }
    else
    {
        /* Initialize texture image with default color */
        auto image = GetDefaultTextureImageRGBAub(size);
        GLTexImage1D(format, size, GL_RGBA, GL_UNSIGNED_BYTE, image.data());
    }
}

void GLRenderSystem::SetupTexture2D(Texture& texture, const TextureFormat format, const Gs::Vector2i& size, const ImageDescriptor* imageDesc)
{
    /* Bind texture and set type */
    auto& textureGL = LLGL_CAST(GLTexture&, texture);
    BindTextureAndSetType(textureGL, TextureType::Texture2D);

    if (imageDesc)
    {
        /* Setup texture image from descriptor */
        GLTexImage2D(format, size.x, size.y, GLTypes::Map(imageDesc->format), GLTypes::Map(imageDesc->dataType), imageDesc->buffer, imageDesc->compressedSize);
    }
    else if (IsCompressedFormat(format))
    {
        /* Initialize compressed texture image with null pointer */
        GLTexImage2D(format, size.x, size.y, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    }
    else
    {
        /* Initialize texture image with default color */
        auto image = GetDefaultTextureImageRGBAub(size.x*size.y);
        GLTexImage2D(format, size.x, size.y, GL_RGBA, GL_UNSIGNED_BYTE, image.data());
    }
}

void GLRenderSystem::SetupTexture3D(Texture& texture, const TextureFormat format, const Gs::Vector3i& size, const ImageDescriptor* imageDesc)
{
    LLGL_ASSERT_CAP(has3DTextures);

    /* Bind texture and set type */
    auto& textureGL = LLGL_CAST(GLTexture&, texture);
    BindTextureAndSetType(textureGL, TextureType::Texture3D);

    if (imageDesc)
    {
        /* Setup texture image from descriptor */
        GLTexImage3D(format, size.x, size.y, size.z, GLTypes::Map(imageDesc->format), GLTypes::Map(imageDesc->dataType), imageDesc->buffer, imageDesc->compressedSize);
    }
    else if (IsCompressedFormat(format))
    {
        /* Initialize compressed texture image with null pointer */
        GLTexImage3D(format, size.x, size.y, size.z, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    }
    else
    {
        /* Initialize texture image with default color */
        auto image = GetDefaultTextureImageRGBAub(size.x*size.y*size.z);
        GLTexImage3D(format, size.x, size.y, size.z, GL_RGBA, GL_UNSIGNED_BYTE, image.data());
    }
}

void GLRenderSystem::SetupTextureCube(Texture& texture, const TextureFormat format, const Gs::Vector2i& size, const ImageDescriptor* imageDesc)
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

    /* Bind texture and set type */
    auto& textureGL = LLGL_CAST(GLTexture&, texture);
    BindTextureAndSetType(textureGL, TextureType::TextureCube);

    if (imageDesc)
    {
        /* Setup texture image cube-faces from descriptor */
        auto imageFace          = reinterpret_cast<const char*>(imageDesc->buffer);
        auto imageFaceStride    = (size.x * size.y * ImageFormatSize(imageDesc->format) * DataTypeSize(imageDesc->dataType));

        if (IsCompressedFormat(format))
            imageFaceStride = imageDesc->compressedSize;

        auto dataFormatGL       = GLTypes::Map(imageDesc->format);
        auto dataTypeGL         = GLTypes::Map(imageDesc->dataType);

        for (auto face : cubeFaces)
        {
            GLTexImageCube(format, size.x, size.y, face, dataFormatGL, dataTypeGL, imageFace, imageDesc->compressedSize);
            imageFace += imageFaceStride;
        }
    }
    else if (IsCompressedFormat(format))
    {
        /* Initialize compressed texture image with null pointer */
        for (auto face : cubeFaces)
            GLTexImageCube(format, size.x, size.y, face, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    }
    else
    {
        /* Initialize texture image cube-faces with default color */
        auto image = GetDefaultTextureImageRGBAub(size.x*size.y);
        for (auto face : cubeFaces)
            GLTexImageCube(format, size.x, size.y, face, GL_RGBA, GL_UNSIGNED_BYTE, image.data());
    }
}

void GLRenderSystem::SetupTexture1DArray(Texture& texture, const TextureFormat format, int size, unsigned int layers, const ImageDescriptor* imageDesc)
{
    LLGL_ASSERT_CAP(hasTextureArrays);

    /* Bind texture and set type */
    auto& textureGL = LLGL_CAST(GLTexture&, texture);
    BindTextureAndSetType(textureGL, TextureType::Texture1DArray);

    if (imageDesc)
    {
        /* Setup texture image from descriptor */
        GLTexImage1DArray(format, size, layers, GLTypes::Map(imageDesc->format), GLTypes::Map(imageDesc->dataType), imageDesc->buffer, imageDesc->compressedSize);
    }
    else if (IsCompressedFormat(format))
    {
        /* Initialize compressed texture image with null pointer */
        GLTexImage1DArray(format, size, layers, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    }
    else
    {
        /* Initialize texture image with default color */
        auto image = GetDefaultTextureImageRGBAub(size*static_cast<int>(layers));
        GLTexImage1DArray(format, size, layers, GL_RGBA, GL_UNSIGNED_BYTE, image.data());
    }
}

void GLRenderSystem::SetupTexture2DArray(Texture& texture, const TextureFormat format, const Gs::Vector2i& size, unsigned int layers, const ImageDescriptor* imageDesc)
{
    LLGL_ASSERT_CAP(hasTextureArrays);

    /* Bind texture and set type */
    auto& textureGL = LLGL_CAST(GLTexture&, texture);
    BindTextureAndSetType(textureGL, TextureType::Texture2DArray);

    if (imageDesc)
    {
        /* Setup texture image from descriptor */
        GLTexImage2DArray(format, size.x, size.y, layers, GLTypes::Map(imageDesc->format), GLTypes::Map(imageDesc->dataType), imageDesc->buffer, imageDesc->compressedSize);
    }
    else if (IsCompressedFormat(format))
    {
        /* Initialize compressed texture image with null pointer */
        GLTexImage2DArray(format, size.x, size.y, layers, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    }
    else
    {
        /* Initialize texture image with default color */
        auto image = GetDefaultTextureImageRGBAub(size.x*size.y*static_cast<int>(layers));
        GLTexImage2DArray(format, size.x, size.y, layers, GL_RGBA, GL_UNSIGNED_BYTE, image.data());
    }
}

void GLRenderSystem::SetupTextureCubeArray(Texture& texture, const TextureFormat format, const Gs::Vector2i& size, unsigned int layers, const ImageDescriptor* imageDesc)
{
    LLGL_ASSERT_CAP(hasCubeTextureArrays);

    /* Bind texture and set type */
    auto& textureGL = LLGL_CAST(GLTexture&, texture);
    BindTextureAndSetType(textureGL, TextureType::TextureCubeArray);

    if (imageDesc)
    {
        /* Setup texture image cube-faces from descriptor */
        GLTexImageCubeArray(format, size.x, size.y, layers, GLTypes::Map(imageDesc->format), GLTypes::Map(imageDesc->dataType), imageDesc->buffer, imageDesc->compressedSize);
    }
    else if (IsCompressedFormat(format))
    {
        /* Initialize compressed texture image with null pointer */
        GLTexImageCubeArray(format, size.x, size.y, layers, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    }
    else
    {
        /* Initialize texture image cube-faces with default color */
        auto image = GetDefaultTextureImageRGBAub(size.x*size.y*static_cast<int>(layers*6));
        GLTexImageCubeArray(format, size.x, size.y, layers, GL_RGBA, GL_UNSIGNED_BYTE, image.data());
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
