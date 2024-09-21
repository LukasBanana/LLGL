/*
 * GLTextureSubImage.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "GLTextureSubImage.h"
#include "../GLTypes.h"
#include "../Ext/GLExtensions.h"
#include "../../../Core/Assertion.h"
#include <array>
#include <algorithm>


namespace LLGL
{


#if LLGL_GLEXT_DIRECT_STATE_ACCESS

static void GLTextureSubImage1DBase(
    GLuint              texID,
    std::uint32_t       mipLevel,
    std::int32_t        x,
    std::uint32_t       width,
    const ImageView&    imageView,
    GLenum              internalFormat)
{
    LLGL_ASSERT(internalFormat != 0);
    if (IsCompressedFormat(imageView.format))
    {
        glCompressedTextureSubImage1D(
            texID,
            static_cast<GLint>(mipLevel),
            x,
            static_cast<GLsizei>(width),
            internalFormat,
            static_cast<GLsizei>(imageView.dataSize),
            imageView.data
        );
    }
    else
    {
        glTextureSubImage1D(
            texID,
            static_cast<GLint>(mipLevel),
            x,
            static_cast<GLsizei>(width),
            GLTypes::Map(imageView.format, GLTypes::IsIntegerTypedFormat(internalFormat)),
            GLTypes::Map(imageView.dataType),
            imageView.data
        );
    }
}

static void GLTextureSubImage2DBase(
    GLuint              texID,
    std::uint32_t       mipLevel,
    std::int32_t        x,
    std::int32_t        y,
    std::uint32_t       width,
    std::uint32_t       height,
    const ImageView&    imageView,
    GLenum              internalFormat)
{
    LLGL_ASSERT(internalFormat != 0);
    if (IsCompressedFormat(imageView.format))
    {
        glCompressedTextureSubImage2D(
            texID,
            static_cast<GLint>(mipLevel),
            x,
            y,
            static_cast<GLsizei>(width),
            static_cast<GLsizei>(height),
            internalFormat,
            static_cast<GLsizei>(imageView.dataSize),
            imageView.data
        );
    }
    else
    {
        glTextureSubImage2D(
            texID,
            static_cast<GLint>(mipLevel),
            x,
            y,
            static_cast<GLsizei>(width),
            static_cast<GLsizei>(height),
            GLTypes::Map(imageView.format, GLTypes::IsIntegerTypedFormat(internalFormat)),
            GLTypes::Map(imageView.dataType),
            imageView.data
        );
    }
}

static void GLTextureSubImage3DBase(
    GLuint              texID,
    std::uint32_t       mipLevel,
    std::int32_t        x,
    std::int32_t        y,
    std::int32_t        z,
    std::uint32_t       width,
    std::uint32_t       height,
    std::uint32_t       depth,
    const ImageView&    imageView,
    GLenum              internalFormat)
{
    LLGL_ASSERT(internalFormat != 0);
    if (IsCompressedFormat(imageView.format))
    {
        glCompressedTextureSubImage3D(
            texID,
            static_cast<GLint>(mipLevel),
            x,
            y,
            z,
            static_cast<GLsizei>(width),
            static_cast<GLsizei>(height),
            static_cast<GLsizei>(depth),
            internalFormat,
            static_cast<GLsizei>(imageView.dataSize),
            imageView.data
        );
    }
    else
    {
        glTextureSubImage3D(
            texID,
            static_cast<GLint>(mipLevel),
            x,
            y,
            z,
            static_cast<GLsizei>(width),
            static_cast<GLsizei>(height),
            static_cast<GLsizei>(depth),
            GLTypes::Map(imageView.format, GLTypes::IsIntegerTypedFormat(internalFormat)),
            GLTypes::Map(imageView.dataType),
            imageView.data
        );
    }
}

static void GLTextureSubImage1D(GLuint texID, const TextureRegion& region, const ImageView& imageView, GLenum internalFormat)
{
    GLTextureSubImage1DBase(
        texID,
        region.subresource.baseMipLevel,
        region.offset.x,
        region.extent.width,
        imageView,
        internalFormat
    );
}

static void GLTextureSubImage2D(GLuint texID, const TextureRegion& region, const ImageView& imageView, GLenum internalFormat)
{
    GLTextureSubImage2DBase(
        texID,
        region.subresource.baseMipLevel,
        region.offset.x,
        region.offset.y,
        region.extent.width,
        region.extent.height,
        imageView,
        internalFormat
    );
}

static void GLTextureSubImage3D(GLuint texID, const TextureRegion& region, const ImageView& imageView, GLenum internalFormat)
{
    GLTextureSubImage3DBase(
        texID,
        region.subresource.baseMipLevel,
        region.offset.x,
        region.offset.y,
        region.offset.z,
        region.extent.width,
        region.extent.height,
        region.extent.depth,
        imageView,
        internalFormat
    );
}

static void GLTextureSubImageCube(GLuint texID, const TextureRegion& region, const ImageView& imageView, GLenum internalFormat)
{
    GLTextureSubImage3DBase(
        texID,
        region.subresource.baseMipLevel,
        region.offset.x,
        region.offset.y,
        region.subresource.baseArrayLayer,
        region.extent.width,
        region.extent.height,
        1,
        imageView,
        internalFormat
    );
}

static void GLTextureSubImage1DArray(GLuint texID, const TextureRegion& region, const ImageView& imageView, GLenum internalFormat)
{
    GLTextureSubImage2DBase(
        texID,
        region.subresource.baseMipLevel,
        region.offset.x,
        region.subresource.baseArrayLayer,
        region.extent.width,
        region.subresource.numArrayLayers,
        imageView,
        internalFormat
    );
}

static void GLTextureSubImage2DArray(GLuint texID, const TextureRegion& region, const ImageView& imageView, GLenum internalFormat)
{
    GLTextureSubImage3DBase(
        texID,
        region.subresource.baseMipLevel,
        region.offset.x,
        region.offset.y,
        region.subresource.baseArrayLayer,
        region.extent.width,
        region.extent.height,
        region.subresource.numArrayLayers,
        imageView,
        internalFormat
    );
}

static void GLTextureSubImageCubeArray(GLuint texID, const TextureRegion& region, const ImageView& imageView, GLenum internalFormat)
{
    GLTextureSubImage3DBase(
        texID,
        region.subresource.baseMipLevel,
        region.offset.x,
        region.offset.y,
        region.subresource.baseArrayLayer,
        region.extent.width,
        region.extent.height,
        region.subresource.numArrayLayers,
        imageView,
        internalFormat
    );
}

#endif // /LLGL_GLEXT_DIRECT_STATE_ACCESS

void GLTextureSubImage(
    GLuint                  texID,
    const TextureType       type,
    const TextureRegion&    region,
    const ImageView&        imageView,
    GLenum                  internalFormat)
{
    #if LLGL_GLEXT_DIRECT_STATE_ACCESS
    switch (type)
    {
        case TextureType::Texture1D:
            GLTextureSubImage1D(texID, region, imageView, internalFormat);
            break;

        case TextureType::Texture2D:
            GLTextureSubImage2D(texID, region, imageView, internalFormat);
            break;

        case TextureType::Texture3D:
            GLTextureSubImage3D(texID, region, imageView, internalFormat);
            break;

        case TextureType::TextureCube:
            GLTextureSubImageCube(texID, region, imageView, internalFormat);
            break;

        case TextureType::Texture1DArray:
            GLTextureSubImage1DArray(texID, region, imageView, internalFormat);
            break;

        case TextureType::Texture2DArray:
            GLTextureSubImage2DArray(texID, region, imageView, internalFormat);
            break;

        case TextureType::TextureCubeArray:
            GLTextureSubImageCubeArray(texID, region, imageView, internalFormat);
            break;

        default:
            break;
    }
    #endif // /LLGL_GLEXT_DIRECT_STATE_ACCESS
}


} // /namespace LLGL



// ================================================================================
