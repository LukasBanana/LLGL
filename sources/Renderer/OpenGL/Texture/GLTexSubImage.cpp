/*
 * GLTexSubImage.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "GLTexSubImage.h"
#include "../Profile/GLProfile.h"
#include "../GLTypes.h"
#include "../Ext/GLExtensions.h"
#include "../Ext/GLExtensionRegistry.h"
#include "../../../Core/Assertion.h"
#include <array>
#include <algorithm>


namespace LLGL
{


#ifdef LLGL_OPENGL

static void GLTexSubImage1DBase(
    GLenum              target,
    std::uint32_t       mipLevel,
    std::int32_t        x,
    std::uint32_t       width,
    const ImageView&    imageView,
    GLenum              internalFormat)
{
    LLGL_ASSERT(internalFormat != 0);
    if (IsCompressedFormat(imageView.format))
    {
        glCompressedTexSubImage1D(
            target,
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
        glTexSubImage1D(
            target,
            static_cast<GLint>(mipLevel),
            x,
            static_cast<GLsizei>(width),
            GLTypes::Map(imageView.format, GLTypes::IsIntegerTypedFormat(internalFormat)),
            GLTypes::Map(imageView.dataType),
            imageView.data
        );
    }
}

#endif

static void GLTexSubImage2DBase(
    GLenum              target,
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
        glCompressedTexSubImage2D(
            target,
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
        glTexSubImage2D(
            target,
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

static void GLTexSubImage3DBase(
    GLenum              target,
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
        glCompressedTexSubImage3D(
            target,
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
        glTexSubImage3D(
            target,
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

#ifdef LLGL_OPENGL

static void GLTexSubImage1D(
    std::uint32_t       mipLevel,
    std::int32_t        x,
    std::uint32_t       width,
    const ImageView&    imageView,
    GLenum              internalFormat)
{
    GLTexSubImage1DBase(
        GL_TEXTURE_1D,
        mipLevel,
        x,
        width,
        imageView,
        internalFormat
    );
}

#endif

static void GLTexSubImage2D(
    std::uint32_t       mipLevel,
    std::int32_t        x,
    std::int32_t        y,
    std::uint32_t       width,
    std::uint32_t       height,
    const ImageView&    imageView,
    GLenum              internalFormat)
{
    GLTexSubImage2DBase(
        GL_TEXTURE_2D,
        mipLevel,
        x,
        y,
        width,
        height,
        imageView,
        internalFormat
    );
}

static void GLTexSubImage3D(
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
    GLTexSubImage3DBase(
        GL_TEXTURE_3D,
        mipLevel,
        x,
        y,
        z,
        width,
        height,
        depth,
        imageView,
        internalFormat
    );
}

static void GLTexSubImageCube(
    std::uint32_t       mipLevel,
    std::int32_t        x,
    std::int32_t        y,
    std::uint32_t       width,
    std::uint32_t       height,
    std::uint32_t       cubeFaceIndex,
    const ImageView&    imageView,
    GLenum              internalFormat)
{
    GLTexSubImage2DBase(
        GLTypes::ToTextureCubeMap(cubeFaceIndex),
        mipLevel,
        x,
        y,
        width,
        height,
        imageView,
        internalFormat
    );
}

#ifdef LLGL_OPENGL

static void GLTexSubImage1DArray(
    std::uint32_t       mipLevel,
    std::int32_t        x,
    std::uint32_t       firstLayer,
    std::uint32_t       width,
    std::uint32_t       numLayers,
    const ImageView&    imageView,
    GLenum              internalFormat)
{
    GLTexSubImage2DBase(
        GL_TEXTURE_1D_ARRAY,
        mipLevel,
        x,
        static_cast<std::int32_t>(firstLayer),
        width,
        numLayers,
        imageView,
        internalFormat
    );
}

#endif

static void GLTexSubImage2DArray(
    std::uint32_t       mipLevel,
    std::int32_t        x,
    std::int32_t        y,
    std::uint32_t       firstLayer,
    std::uint32_t       width,
    std::uint32_t       height,
    std::uint32_t       numLayers,
    const ImageView&    imageView,
    GLenum              internalFormat)
{
    GLTexSubImage3DBase(
        GL_TEXTURE_2D_ARRAY,
        mipLevel,
        x,
        y,
        static_cast<std::int32_t>(firstLayer),
        width,
        height,
        numLayers,
        imageView,
        internalFormat
    );
}

#ifdef LLGL_OPENGL

static void GLTexSubImageCubeArray(
    std::uint32_t       mipLevel,
    std::int32_t        x,
    std::int32_t        y,
    std::uint32_t       firstLayer,
    std::uint32_t       width,
    std::uint32_t       height,
    std::uint32_t       numLayers,
    const ImageView&    imageView,
    GLenum              internalFormat)
{
    GLTexSubImage3DBase(
        GL_TEXTURE_CUBE_MAP_ARRAY,
        mipLevel,
        x,
        y,
        static_cast<std::int32_t>(firstLayer),
        width,
        height,
        numLayers,
        imageView,
        internalFormat
    );
}

static void GLTexSubImage1D(const TextureRegion& region, const ImageView& imageView, GLenum internalFormat)
{
    GLTexSubImage1D(
        region.subresource.baseMipLevel,
        region.offset.x,
        region.extent.width,
        imageView,
        internalFormat
    );
}

#endif

static void GLTexSubImage2D(const TextureRegion& region, const ImageView& imageView, GLenum internalFormat)
{
    GLTexSubImage2D(
        region.subresource.baseMipLevel,
        region.offset.x,
        region.offset.y,
        region.extent.width,
        region.extent.height,
        imageView,
        internalFormat
    );
}

static void GLTexSubImage3D(const TextureRegion& region, const ImageView& imageView, GLenum internalFormat)
{
    GLTexSubImage3D(
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

static void GLTexSubImageCube(const TextureRegion& region, const ImageView& imageView, GLenum internalFormat)
{
    GLTexSubImageCube(
        region.subresource.baseMipLevel,
        region.offset.x,
        region.offset.y,
        region.extent.width,
        region.extent.height,
        region.subresource.baseArrayLayer,
        imageView,
        internalFormat
    );
}

#ifdef LLGL_OPENGL

static void GLTexSubImage1DArray(const TextureRegion& region, const ImageView& imageView, GLenum internalFormat)
{
    GLTexSubImage1DArray(
        region.subresource.baseMipLevel,
        region.offset.x,
        region.subresource.baseArrayLayer,
        region.extent.width,
        region.subresource.numArrayLayers,
        imageView,
        internalFormat
    );
}

#endif

static void GLTexSubImage2DArray(const TextureRegion& region, const ImageView& imageView, GLenum internalFormat)
{
    GLTexSubImage2DArray(
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

#ifdef LLGL_OPENGL

static void GLTexSubImageCubeArray(const TextureRegion& region, const ImageView& imageView, GLenum internalFormat)
{
    GLTexSubImageCubeArray(
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

#endif

bool GLTexSubImage(
    const TextureType       type,
    const TextureRegion&    region,
    const ImageView&        imageView,
    GLenum                  internalFormat)
{
    //TODO: on-the-fly decompression would be awesome (if GL_ARB_texture_compression is unsupported), but a lot of work :-/
    /* If compressed format is requested, GL_ARB_texture_compression must be supported */
    if (IsCompressedFormat(imageView.format) && !HasExtension(GLExt::ARB_texture_compression))
        return false;

    switch (type)
    {
        #ifdef LLGL_OPENGL
        case TextureType::Texture1D:
            GLTexSubImage1D(region, imageView, internalFormat);
            break;
        #endif

        case TextureType::Texture2D:
            GLTexSubImage2D(region, imageView, internalFormat);
            break;

        case TextureType::Texture3D:
            GLTexSubImage3D(region, imageView, internalFormat);
            break;

        case TextureType::TextureCube:
            GLTexSubImageCube(region, imageView, internalFormat);
            break;

        #ifdef LLGL_OPENGL
        case TextureType::Texture1DArray:
            GLTexSubImage1DArray(region, imageView, internalFormat);
            break;
        #endif

        case TextureType::Texture2DArray:
            GLTexSubImage2DArray(region, imageView, internalFormat);
            break;

        #ifdef LLGL_OPENGL
        case TextureType::TextureCubeArray:
            GLTexSubImageCubeArray(region, imageView, internalFormat);
            break;
        #endif

        default:
            return false;
    }

    return true;
}


} // /namespace LLGL



// ================================================================================
