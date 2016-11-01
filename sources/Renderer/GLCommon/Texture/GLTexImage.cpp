/*
 * GLTexImage.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLTexImage.h"


namespace LLGL
{


#if 0

static void GLTexImage1DBase(
    GLenum target, const TextureFormat internalFormat, unsigned int width,
    GLenum format, GLenum type, const void* data, unsigned int compressedSize)
{
    if (IsCompressedFormat(internalFormat))
    {
        glCompressedTexImage1D(
            target, 0, GLTypes::Map(internalFormat),
            static_cast<GLsizei>(width),
            0, static_cast<GLsizei>(compressedSize), data
        );
    }
    else
    {
        glTexImage1D(
            target, 0, GLTypes::Map(internalFormat),
            static_cast<GLsizei>(width),
            0, format, type, data
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
            static_cast<GLsizei>(width),
            static_cast<GLsizei>(height),
            0, static_cast<GLsizei>(compressedSize), data
        );
    }
    else
    {
        glTexImage2D(
            target, 0, GLTypes::Map(internalFormat),
            static_cast<GLsizei>(width),
            static_cast<GLsizei>(height),
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
            static_cast<GLsizei>(width),
            static_cast<GLsizei>(height),
            static_cast<GLsizei>(depth),
            0, static_cast<GLsizei>(compressedSize), data
        );
    }
    else
    {
        glTexImage3D(
            target, 0, GLTypes::Map(internalFormat),
            static_cast<GLsizei>(width),
            static_cast<GLsizei>(height),
            static_cast<GLsizei>(depth),
            0, format, type, data
        );
    }
}

static void GLTexImage2DMultisampleBase(
    GLenum target, unsigned int samples, const TextureFormat internalFormat, unsigned int width, unsigned int height, bool fixedSamples)
{
    glTexImage2DMultisample(
        target,
        static_cast<GLsizei>(samples),
        GLTypes::Map(internalFormat),
        static_cast<GLsizei>(width),
        static_cast<GLsizei>(height),
        (fixedSamples ? GL_TRUE : GL_FALSE)
    );
}

static void GLTexImage3DMultisampleBase(
    GLenum target, unsigned int samples, const TextureFormat internalFormat, unsigned int width, unsigned int height, unsigned int depth, bool fixedSamples)
{
    glTexImage3DMultisample(
        target,
        static_cast<GLsizei>(samples),
        GLTypes::Map(internalFormat),
        static_cast<GLsizei>(width),
        static_cast<GLsizei>(height),
        static_cast<GLsizei>(depth),
        (fixedSamples ? GL_TRUE : GL_FALSE)
    );
}

static void GLTexImage1D(
    const TextureFormat internalFormat, unsigned int width,
    GLenum format, GLenum type, const void* data, unsigned int compressedSize = 0)
{
    GLTexImage1DBase(GL_TEXTURE_1D, internalFormat, width, format, type, data, compressedSize);
}

static void GLTexImage2D(
    const TextureFormat internalFormat, unsigned int width, unsigned int height,
    GLenum format, GLenum type, const void* data, unsigned int compressedSize = 0)
{
    GLTexImage2DBase(GL_TEXTURE_2D, internalFormat, width, height, format, type, data, compressedSize);
}

static void GLTexImage3D(
    const TextureFormat internalFormat, unsigned int width, unsigned int height, unsigned int depth,
    GLenum format, GLenum type, const void* data, unsigned int compressedSize = 0)
{
    GLTexImage3DBase(GL_TEXTURE_3D, internalFormat, width, height, depth, format, type, data, compressedSize);
}

static void GLTexImageCube(
    const TextureFormat internalFormat, unsigned int width, unsigned int height, AxisDirection cubeFace,
    GLenum format, GLenum type, const void* data, unsigned int compressedSize = 0)
{
    GLTexImage2DBase(GLTypes::Map(cubeFace), internalFormat, width, height, format, type, data, compressedSize);
}

static void GLTexImage1DArray(
    const TextureFormat internalFormat, unsigned int width, unsigned int layers,
    GLenum format, GLenum type, const void* data, unsigned int compressedSize = 0)
{
    GLTexImage2DBase(GL_TEXTURE_1D_ARRAY, internalFormat, width, layers, format, type, data, compressedSize);
}

static void GLTexImage2DArray(
    const TextureFormat internalFormat, unsigned int width, unsigned int height, unsigned int layers,
    GLenum format, GLenum type, const void* data, unsigned int compressedSize = 0)
{
    GLTexImage3DBase(GL_TEXTURE_2D_ARRAY, internalFormat, width, height, layers, format, type, data, compressedSize);
}

static void GLTexImageCubeArray(
    const TextureFormat internalFormat, unsigned int width, unsigned int height, unsigned int layers,
    GLenum format, GLenum type, const void* data, unsigned int compressedSize = 0)
{
    GLTexImage3DBase(GL_TEXTURE_CUBE_MAP_ARRAY, internalFormat, width, height, layers*6, format, type, data, compressedSize);
}

static void GLTexImage2DMultisample(
    unsigned int samples, const TextureFormat internalFormat, unsigned int width, unsigned int height, bool fixedSamples)
{
    GLTexImage2DMultisampleBase(GL_TEXTURE_2D_MULTISAMPLE, samples, internalFormat, width, height, fixedSamples);
}

static void GLTexImage2DMultisampleArray(
    unsigned int samples, const TextureFormat internalFormat, unsigned int width, unsigned int height, unsigned int depth, bool fixedSamples)
{
    GLTexImage3DMultisampleBase(GL_TEXTURE_2D_MULTISAMPLE_ARRAY, samples, internalFormat, width, height, depth, fixedSamples);
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

#endif


} // /namespace LLGL



// ================================================================================
