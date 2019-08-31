/*
 * GLTexSubImage.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLTexSubImage.h"
#include "../GLTypes.h"
#include "../GLImport.h"
#include "../GLImportExt.h"
#include <array>
#include <algorithm>


namespace LLGL
{


#ifdef LLGL_OPENGL

static void GLTexSubImage1DBase(
    GLenum                      target,
    std::uint32_t               mipLevel,
    std::int32_t                x,
    std::uint32_t               width,
    const SrcImageDescriptor&   imageDesc)
{
    if (IsCompressedFormat(imageDesc.format))
    {
        glCompressedTexSubImage1D(
            target,
            static_cast<GLint>(mipLevel),
            x,
            static_cast<GLsizei>(width),
            GLTypes::Map(imageDesc.format),
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
            GLTypes::Map(imageDesc.format),
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
    const SrcImageDescriptor&   imageDesc)
{
    if (IsCompressedFormat(imageDesc.format))
    {
        glCompressedTexSubImage2D(
            target,
            static_cast<GLint>(mipLevel),
            x,
            y,
            static_cast<GLsizei>(width),
            static_cast<GLsizei>(height),
            GLTypes::Map(imageDesc.format),
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
            GLTypes::Map(imageDesc.format),
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
    const SrcImageDescriptor&   imageDesc)
{
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
            GLTypes::Map(imageDesc.format),
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
            GLTypes::Map(imageDesc.format),
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
    const SrcImageDescriptor&   imageDesc)
{
    GLTexSubImage1DBase(GL_TEXTURE_1D, mipLevel, x, width, imageDesc);
}

#endif

static void GLTexSubImage2D(
    std::uint32_t               mipLevel,
    std::int32_t                x,
    std::int32_t                y,
    std::uint32_t               width,
    std::uint32_t               height,
    const SrcImageDescriptor&   imageDesc)
{
    GLTexSubImage2DBase(GL_TEXTURE_2D, mipLevel, x, y, width, height, imageDesc);
}

static void GLTexSubImage3D(
    std::uint32_t               mipLevel,
    std::int32_t                x,
    std::int32_t                y,
    std::int32_t                z,
    std::uint32_t               width,
    std::uint32_t               height,
    std::uint32_t               depth,
    const SrcImageDescriptor&   imageDesc)
{
    GLTexSubImage3DBase(GL_TEXTURE_3D, mipLevel, x, y, z, width, height, depth, imageDesc);
}

static void GLTexSubImageCube(
    std::uint32_t               mipLevel,
    std::int32_t                x,
    std::int32_t                y,
    std::uint32_t               width,
    std::uint32_t               height,
    std::uint32_t               cubeFaceIndex,
    const SrcImageDescriptor&   imageDesc)
{
    GLTexSubImage2DBase(GLTypes::ToTextureCubeMap(cubeFaceIndex), mipLevel, x, y, width, height, imageDesc);
}

#ifdef LLGL_OPENGL

static void GLTexSubImage1DArray(
    std::uint32_t               mipLevel,
    std::int32_t                x,
    std::uint32_t               firstLayer,
    std::uint32_t               width,
    std::uint32_t               numLayers,
    const SrcImageDescriptor&   imageDesc)
{
    GLTexSubImage2DBase(GL_TEXTURE_1D_ARRAY, mipLevel, x, static_cast<std::int32_t>(firstLayer), width, numLayers, imageDesc);
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
    const SrcImageDescriptor&   imageDesc)
{
    GLTexSubImage3DBase(GL_TEXTURE_2D_ARRAY, mipLevel, x, y, static_cast<std::int32_t>(firstLayer), width, height, numLayers, imageDesc);
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
    const SrcImageDescriptor&   imageDesc)
{
    GLTexSubImage3DBase(GL_TEXTURE_CUBE_MAP_ARRAY, mipLevel, x, y, static_cast<std::int32_t>(firstLayer), width, height, numLayers, imageDesc);
}

void GLTexSubImage1D(const TextureRegion& region, const SrcImageDescriptor& imageDesc)
{
    GLTexSubImage1D(
        region.subresource.baseMipLevel,
        region.offset.x,
        region.extent.width,
        imageDesc
    );
}

#endif

void GLTexSubImage2D(const TextureRegion& region, const SrcImageDescriptor& imageDesc)
{
    GLTexSubImage2D(
        region.subresource.baseMipLevel,
        region.offset.x,
        region.offset.y,
        region.extent.width,
        region.extent.height,
        imageDesc
    );
}

void GLTexSubImage3D(const TextureRegion& region, const SrcImageDescriptor& imageDesc)
{
    GLTexSubImage3D(
        region.subresource.baseMipLevel,
        region.offset.x,
        region.offset.y,
        region.offset.z,
        region.extent.width,
        region.extent.height,
        region.extent.depth,
        imageDesc
    );
}

void GLTexSubImageCube(const TextureRegion& region, const SrcImageDescriptor& imageDesc)
{
    GLTexSubImageCube(
        region.subresource.baseMipLevel,
        region.offset.x,
        region.offset.y,
        region.extent.width,
        region.extent.height,
        region.subresource.baseArrayLayer,
        imageDesc
    );
}

#ifdef LLGL_OPENGL

void GLTexSubImage1DArray(const TextureRegion& region, const SrcImageDescriptor& imageDesc)
{
    GLTexSubImage1DArray(
        region.subresource.baseMipLevel,
        region.offset.x,
        region.subresource.baseArrayLayer,
        region.extent.width,
        region.subresource.numArrayLayers,
        imageDesc
    );
}

#endif

void GLTexSubImage2DArray(const TextureRegion& region, const SrcImageDescriptor& imageDesc)
{
    GLTexSubImage2DArray(
        region.subresource.baseMipLevel,
        region.offset.x,
        region.offset.y,
        region.subresource.baseArrayLayer,
        region.extent.width,
        region.extent.height,
        region.subresource.numArrayLayers,
        imageDesc
    );
}

#ifdef LLGL_OPENGL

void GLTexSubImageCubeArray(const TextureRegion& region, const SrcImageDescriptor& imageDesc)
{
    GLTexSubImageCubeArray(
        region.subresource.baseMipLevel,
        region.offset.x,
        region.offset.y,
        region.subresource.baseArrayLayer,
        region.extent.width,
        region.extent.height,
        region.subresource.numArrayLayers,
        imageDesc
    );
}

#endif


} // /namespace LLGL



// ================================================================================
