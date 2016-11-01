/*
 * GLTexSubImage.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLTexSubImage.h"
#include "../GLTypes.h"
#include "../OpenGL.h"
#include "../OpenGLExt.h"
#include <array>


namespace LLGL
{


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

void GLWriteTexture1D(const SubTextureDescriptor& desc, const ImageDescriptor& imageDesc)
{
    GLTexSubImage1D(desc.mipLevel, desc.texture1D.x, desc.texture1D.width, imageDesc);
}

void GLWriteTexture2D(const SubTextureDescriptor& desc, const ImageDescriptor& imageDesc)
{
    GLTexSubImage2D(
        desc.mipLevel, desc.texture2D.x, desc.texture2D.y,
        desc.texture2D.width, desc.texture2D.height, imageDesc
    );
}

void GLWriteTexture3D(const SubTextureDescriptor& desc, const ImageDescriptor& imageDesc)
{
    GLTexSubImage3D(
        desc.mipLevel, desc.texture3D.x, desc.texture3D.y, desc.texture3D.z,
        desc.texture3D.width, desc.texture3D.height, desc.texture3D.depth, imageDesc
    );
}

void GLWriteTextureCube(const SubTextureDescriptor& desc, const ImageDescriptor& imageDesc)
{
    GLTexSubImageCube(
        desc.mipLevel, desc.textureCube.x, desc.textureCube.y,
        desc.textureCube.width, desc.textureCube.height, desc.textureCube.cubeFaceOffset, imageDesc
    );
}

void GLWriteTexture1DArray(const SubTextureDescriptor& desc, const ImageDescriptor& imageDesc)
{
    GLTexSubImage1DArray(
        desc.mipLevel, desc.texture1D.x, desc.texture1D.layerOffset,
        desc.texture1D.width, desc.texture1D.layers, imageDesc
    );
}

void GLWriteTexture2DArray(const SubTextureDescriptor& desc, const ImageDescriptor& imageDesc)
{
    GLTexSubImage2DArray(
        desc.mipLevel, desc.texture2D.x, desc.texture2D.y, desc.texture2D.layerOffset,
        desc.texture2D.width, desc.texture2D.height, desc.texture2D.layers, imageDesc
    );
}

void GLWriteTextureCubeArray(const SubTextureDescriptor& desc, const ImageDescriptor& imageDesc)
{
    GLTexSubImageCubeArray(
        desc.mipLevel, desc.textureCube.x, desc.textureCube.y, desc.textureCube.layerOffset, desc.textureCube.cubeFaceOffset,
        desc.textureCube.width, desc.textureCube.height, desc.textureCube.cubeFaces, imageDesc
    );
}


} // /namespace LLGL



// ================================================================================
