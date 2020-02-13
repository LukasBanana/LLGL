/*
 * GLTexSubImage.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLTexSubImage.h"
#include "../GLProfile.h"
#include "../GLTypes.h"
#include "../Ext/GLExtensions.h"
#include <array>
#include <algorithm>


namespace LLGL
{


static void QueryGLInternalFormat(GLenum target, GLenum& internalFormat)
{
    if (internalFormat == 0)
    {
        GLint format = 0;
        GLProfile::GetTexParameterInternalFormat(target, &format);
        internalFormat = static_cast<GLenum>(format);
    }
}

#ifdef LLGL_OPENGL

static void GLTexSubImage1DBase(
    GLenum                      target,
    std::uint32_t               mipLevel,
    std::int32_t                x,
    std::uint32_t               width,
    const SrcImageDescriptor&   imageDesc,
    GLenum                      internalFormat)
{
    QueryGLInternalFormat(target, internalFormat);
    if (IsCompressedFormat(imageDesc.format))
    {
        glCompressedTexSubImage1D(
            target,
            static_cast<GLint>(mipLevel),
            x,
            static_cast<GLsizei>(width),
            internalFormat,
            static_cast<GLsizei>(imageDesc.dataSize),
            imageDesc.data
        );
    }
    else
    {
        glTexSubImage1D(
            target,
            static_cast<GLint>(mipLevel),
            x,
            static_cast<GLsizei>(width),
            GLTypes::Map(imageDesc.format, GLTypes::IsIntegerTypedFormat(internalFormat)),
            GLTypes::Map(imageDesc.dataType),
            imageDesc.data
        );
    }
}

#endif

static void GLTexSubImage2DBase(
    GLenum                      target,
    std::uint32_t               mipLevel,
    std::int32_t                x,
    std::int32_t                y,
    std::uint32_t               width,
    std::uint32_t               height,
    const SrcImageDescriptor&   imageDesc,
    GLenum                      internalFormat)
{
    QueryGLInternalFormat(target, internalFormat);
    if (IsCompressedFormat(imageDesc.format))
    {
        glCompressedTexSubImage2D(
            target,
            static_cast<GLint>(mipLevel),
            x,
            y,
            static_cast<GLsizei>(width),
            static_cast<GLsizei>(height),
            internalFormat,
            static_cast<GLsizei>(imageDesc.dataSize),
            imageDesc.data
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
            GLTypes::Map(imageDesc.format, GLTypes::IsIntegerTypedFormat(internalFormat)),
            GLTypes::Map(imageDesc.dataType),
            imageDesc.data
        );
    }
}

static void GLTexSubImage3DBase(
    GLenum                      target,
    std::uint32_t               mipLevel,
    std::int32_t                x,
    std::int32_t                y,
    std::int32_t                z,
    std::uint32_t               width,
    std::uint32_t               height,
    std::uint32_t               depth,
    const SrcImageDescriptor&   imageDesc,
    GLenum                      internalFormat)
{
    QueryGLInternalFormat(target, internalFormat);
    if (IsCompressedFormat(imageDesc.format))
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
            static_cast<GLsizei>(imageDesc.dataSize),
            imageDesc.data
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
            GLTypes::Map(imageDesc.format, GLTypes::IsIntegerTypedFormat(internalFormat)),
            GLTypes::Map(imageDesc.dataType),
            imageDesc.data
        );
    }
}

#ifdef LLGL_OPENGL

static void GLTexSubImage1D(
    std::uint32_t               mipLevel,
    std::int32_t                x,
    std::uint32_t               width,
    const SrcImageDescriptor&   imageDesc,
    GLenum                      internalFormat)
{
    GLTexSubImage1DBase(
        GL_TEXTURE_1D,
        mipLevel,
        x,
        width,
        imageDesc,
        internalFormat
    );
}

#endif

static void GLTexSubImage2D(
    std::uint32_t               mipLevel,
    std::int32_t                x,
    std::int32_t                y,
    std::uint32_t               width,
    std::uint32_t               height,
    const SrcImageDescriptor&   imageDesc,
    GLenum                      internalFormat)
{
    GLTexSubImage2DBase(
        GL_TEXTURE_2D,
        mipLevel,
        x,
        y,
        width,
        height,
        imageDesc,
        internalFormat
    );
}

static void GLTexSubImage3D(
    std::uint32_t               mipLevel,
    std::int32_t                x,
    std::int32_t                y,
    std::int32_t                z,
    std::uint32_t               width,
    std::uint32_t               height,
    std::uint32_t               depth,
    const SrcImageDescriptor&   imageDesc,
    GLenum                      internalFormat)
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
        imageDesc,
        internalFormat
    );
}

static void GLTexSubImageCube(
    std::uint32_t               mipLevel,
    std::int32_t                x,
    std::int32_t                y,
    std::uint32_t               width,
    std::uint32_t               height,
    std::uint32_t               cubeFaceIndex,
    const SrcImageDescriptor&   imageDesc,
    GLenum                      internalFormat)
{
    GLTexSubImage2DBase(
        GLTypes::ToTextureCubeMap(cubeFaceIndex),
        mipLevel,
        x,
        y,
        width,
        height,
        imageDesc,
        internalFormat
    );
}

#ifdef LLGL_OPENGL

static void GLTexSubImage1DArray(
    std::uint32_t               mipLevel,
    std::int32_t                x,
    std::uint32_t               firstLayer,
    std::uint32_t               width,
    std::uint32_t               numLayers,
    const SrcImageDescriptor&   imageDesc,
    GLenum                      internalFormat)
{
    GLTexSubImage2DBase(
        GL_TEXTURE_1D_ARRAY,
        mipLevel,
        x,
        static_cast<std::int32_t>(firstLayer),
        width,
        numLayers,
        imageDesc,
        internalFormat
    );
}

#endif

static void GLTexSubImage2DArray(
    std::uint32_t               mipLevel,
    std::int32_t                x,
    std::int32_t                y,
    std::uint32_t               firstLayer,
    std::uint32_t               width,
    std::uint32_t               height,
    std::uint32_t               numLayers,
    const SrcImageDescriptor&   imageDesc,
    GLenum                      internalFormat)
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
        imageDesc,
        internalFormat
    );
}

#ifdef LLGL_OPENGL

static void GLTexSubImageCubeArray(
    std::uint32_t               mipLevel,
    std::int32_t                x,
    std::int32_t                y,
    std::uint32_t               firstLayer,
    std::uint32_t               width,
    std::uint32_t               height,
    std::uint32_t               numLayers,
    const SrcImageDescriptor&   imageDesc,
    GLenum                      internalFormat)
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
        imageDesc,
        internalFormat
    );
}

static void GLTexSubImage1D(const TextureRegion& region, const SrcImageDescriptor& imageDesc, GLenum internalFormat)
{
    GLTexSubImage1D(
        region.subresource.baseMipLevel,
        region.offset.x,
        region.extent.width,
        imageDesc,
        internalFormat
    );
}

#endif

static void GLTexSubImage2D(const TextureRegion& region, const SrcImageDescriptor& imageDesc, GLenum internalFormat)
{
    GLTexSubImage2D(
        region.subresource.baseMipLevel,
        region.offset.x,
        region.offset.y,
        region.extent.width,
        region.extent.height,
        imageDesc,
        internalFormat
    );
}

static void GLTexSubImage3D(const TextureRegion& region, const SrcImageDescriptor& imageDesc, GLenum internalFormat)
{
    GLTexSubImage3D(
        region.subresource.baseMipLevel,
        region.offset.x,
        region.offset.y,
        region.offset.z,
        region.extent.width,
        region.extent.height,
        region.extent.depth,
        imageDesc,
        internalFormat
    );
}

static void GLTexSubImageCube(const TextureRegion& region, const SrcImageDescriptor& imageDesc, GLenum internalFormat)
{
    GLTexSubImageCube(
        region.subresource.baseMipLevel,
        region.offset.x,
        region.offset.y,
        region.extent.width,
        region.extent.height,
        region.subresource.baseArrayLayer,
        imageDesc,
        internalFormat
    );
}

#ifdef LLGL_OPENGL

static void GLTexSubImage1DArray(const TextureRegion& region, const SrcImageDescriptor& imageDesc, GLenum internalFormat)
{
    GLTexSubImage1DArray(
        region.subresource.baseMipLevel,
        region.offset.x,
        region.subresource.baseArrayLayer,
        region.extent.width,
        region.subresource.numArrayLayers,
        imageDesc,
        internalFormat
    );
}

#endif

static void GLTexSubImage2DArray(const TextureRegion& region, const SrcImageDescriptor& imageDesc, GLenum internalFormat)
{
    GLTexSubImage2DArray(
        region.subresource.baseMipLevel,
        region.offset.x,
        region.offset.y,
        region.subresource.baseArrayLayer,
        region.extent.width,
        region.extent.height,
        region.subresource.numArrayLayers,
        imageDesc,
        internalFormat
    );
}

#ifdef LLGL_OPENGL

static void GLTexSubImageCubeArray(const TextureRegion& region, const SrcImageDescriptor& imageDesc, GLenum internalFormat)
{
    GLTexSubImageCubeArray(
        region.subresource.baseMipLevel,
        region.offset.x,
        region.offset.y,
        region.subresource.baseArrayLayer,
        region.extent.width,
        region.extent.height,
        region.subresource.numArrayLayers,
        imageDesc,
        internalFormat
    );
}

#endif

void GLTexSubImage(
    const TextureType           type,
    const TextureRegion&        region,
    const SrcImageDescriptor&   imageDesc,
    GLenum                      internalFormat)
{
    switch (type)
    {
        #ifdef LLGL_OPENGL
        case TextureType::Texture1D:
            GLTexSubImage1D(region, imageDesc, internalFormat);
            break;
        #endif

        case TextureType::Texture2D:
            GLTexSubImage2D(region, imageDesc, internalFormat);
            break;

        case TextureType::Texture3D:
            GLTexSubImage3D(region, imageDesc, internalFormat);
            break;

        case TextureType::TextureCube:
            GLTexSubImageCube(region, imageDesc, internalFormat);
            break;

        #ifdef LLGL_OPENGL
        case TextureType::Texture1DArray:
            GLTexSubImage1DArray(region, imageDesc, internalFormat);
            break;
        #endif

        case TextureType::Texture2DArray:
            GLTexSubImage2DArray(region, imageDesc, internalFormat);
            break;

        #ifdef LLGL_OPENGL
        case TextureType::TextureCubeArray:
            GLTexSubImageCubeArray(region, imageDesc, internalFormat);
            break;
        #endif

        default:
            break;
    }
}


} // /namespace LLGL



// ================================================================================
