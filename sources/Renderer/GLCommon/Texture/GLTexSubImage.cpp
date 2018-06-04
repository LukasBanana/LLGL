/*
 * GLTexSubImage.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLTexSubImage.h"
#include "../GLTypes.h"
#include "../GLImport.h"
#include "../GLImportExt.h"
#include <array>


namespace LLGL
{


#ifdef LLGL_OPENGL

static void GLTexSubImage1DBase(
    GLenum                      target,
    std::uint32_t               mipLevel,
    std::uint32_t               x,
    std::uint32_t               width,
    const SrcImageDescriptor&   imageDesc)
{
    if (IsCompressedFormat(imageDesc.format))
    {
        glCompressedTexSubImage1D(
            target,
            static_cast<GLint>(mipLevel),
            static_cast<GLint>(x),
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
            static_cast<GLint>(x),
            static_cast<GLsizei>(width),
            GLTypes::Map(imageDesc.format),
            GLTypes::Map(imageDesc.dataType),
            imageDesc.data
        );
    }
}

#endif

static void GLTexSubImage2DBase(
    GLenum                    target,
    std::uint32_t             mipLevel,
    std::uint32_t             x,
    std::uint32_t             y,
    std::uint32_t             width,
    std::uint32_t             height,
    const SrcImageDescriptor& imageDesc)
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
            static_cast<GLsizei>(imageDesc.dataSize),
            imageDesc.data
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
            imageDesc.data
        );
    }
}

static void GLTexSubImage3DBase(
    GLenum                    target,
    std::uint32_t             mipLevel,
    std::uint32_t             x,
    std::uint32_t             y,
    std::uint32_t             z,
    std::uint32_t             width,
    std::uint32_t             height,
    std::uint32_t             depth,
    const SrcImageDescriptor& imageDesc)
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
            static_cast<GLsizei>(imageDesc.dataSize),
            imageDesc.data
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
            imageDesc.data
        );
    }
}

#ifdef LLGL_OPENGL

static void GLTexSubImage1D(
    std::uint32_t             mipLevel,
    std::uint32_t             x,
    std::uint32_t             width,
    const SrcImageDescriptor& imageDesc)
{
    GLTexSubImage1DBase(GL_TEXTURE_1D, mipLevel, x, width, imageDesc);
}

#endif

static void GLTexSubImage2D(
    std::uint32_t             mipLevel,
    std::uint32_t             x,
    std::uint32_t             y,
    std::uint32_t             width,
    std::uint32_t             height,
    const SrcImageDescriptor& imageDesc)
{
    GLTexSubImage2DBase(GL_TEXTURE_2D, mipLevel, x, y, width, height, imageDesc);
}

static void GLTexSubImage3D(
    std::uint32_t             mipLevel,
    std::uint32_t             x,
    std::uint32_t             y,
    std::uint32_t             z,
    std::uint32_t             width,
    std::uint32_t             height,
    std::uint32_t             depth,
    const SrcImageDescriptor& imageDesc)
{
    GLTexSubImage3DBase(GL_TEXTURE_3D, mipLevel, x, y, z, width, height, depth, imageDesc);
}

static void GLTexSubImageCube(
    std::uint32_t             mipLevel,
    std::uint32_t             x,
    std::uint32_t             y,
    std::uint32_t             width,
    std::uint32_t             height,
    AxisDirection             cubeFace,
    const SrcImageDescriptor& imageDesc)
{
    GLTexSubImage2DBase(GLTypes::Map(cubeFace), mipLevel, x, y, width, height, imageDesc);
}

#ifdef LLGL_OPENGL

static void GLTexSubImage1DArray(
    std::uint32_t             mipLevel,
    std::uint32_t             x,
    std::uint32_t             layerOffset,
    std::uint32_t             width,
    std::uint32_t             layers,
    const SrcImageDescriptor& imageDesc)
{
    GLTexSubImage2DBase(GL_TEXTURE_1D_ARRAY, mipLevel, x, layerOffset, width, layers, imageDesc);
}

#endif

static void GLTexSubImage2DArray(
    std::uint32_t             mipLevel,
    std::uint32_t             x,
    std::uint32_t             y,
    std::uint32_t             layerOffset,
    std::uint32_t             width,
    std::uint32_t             height,
    std::uint32_t             layers,
    const SrcImageDescriptor& imageDesc)
{
    GLTexSubImage3DBase(GL_TEXTURE_2D_ARRAY, mipLevel, x, y, layerOffset, width, height, layers, imageDesc);
}

#ifdef LLGL_OPENGL

static void GLTexSubImageCubeArray(
    std::uint32_t             mipLevel,
    std::uint32_t             x,
    std::uint32_t             y,
    std::uint32_t             layerOffset,
    AxisDirection             cubeFaceOffset,
    std::uint32_t             width,
    std::uint32_t             height,
    std::uint32_t             cubeFaces,
    const SrcImageDescriptor& imageDesc)
{
    layerOffset = layerOffset * 6 + static_cast<std::uint32_t>(cubeFaceOffset);
    GLTexSubImage3DBase(GL_TEXTURE_CUBE_MAP_ARRAY, mipLevel, x, y, layerOffset, width, height, cubeFaces, imageDesc);
}

void GLTexSubImage1D(const SubTextureDescriptor& desc, const SrcImageDescriptor& imageDesc)
{
    GLTexSubImage1D(
        desc.mipLevel,
        desc.texture1D.x,
        desc.texture1D.width,
        imageDesc
    );
}

#endif

void GLTexSubImage2D(const SubTextureDescriptor& desc, const SrcImageDescriptor& imageDesc)
{
    GLTexSubImage2D(
        desc.mipLevel,
        desc.texture2D.x,
        desc.texture2D.y,
        desc.texture2D.width,
        desc.texture2D.height,
        imageDesc
    );
}

void GLTexSubImage3D(const SubTextureDescriptor& desc, const SrcImageDescriptor& imageDesc)
{
    GLTexSubImage3D(
        desc.mipLevel,
        desc.texture3D.x,
        desc.texture3D.y,
        desc.texture3D.z,
        desc.texture3D.width,
        desc.texture3D.height,
        desc.texture3D.depth,
        imageDesc
    );
}

void GLTexSubImageCube(const SubTextureDescriptor& desc, const SrcImageDescriptor& imageDesc)
{
    GLTexSubImageCube(
        desc.mipLevel,
        desc.textureCube.x,
        desc.textureCube.y,
        desc.textureCube.width,
        desc.textureCube.height,
        desc.textureCube.cubeFaceOffset,
        imageDesc
    );
}

#ifdef LLGL_OPENGL

void GLTexSubImage1DArray(const SubTextureDescriptor& desc, const SrcImageDescriptor& imageDesc)
{
    GLTexSubImage1DArray(
        desc.mipLevel,
        desc.texture1D.x,
        desc.texture1D.layerOffset,
        desc.texture1D.width,
        desc.texture1D.layers,
        imageDesc
    );
}

#endif

void GLTexSubImage2DArray(const SubTextureDescriptor& desc, const SrcImageDescriptor& imageDesc)
{
    GLTexSubImage2DArray(
        desc.mipLevel,
        desc.texture2D.x,
        desc.texture2D.y,
        desc.texture2D.layerOffset,
        desc.texture2D.width,
        desc.texture2D.height,
        desc.texture2D.layers,
        imageDesc
    );
}

#ifdef LLGL_OPENGL

void GLTexSubImageCubeArray(const SubTextureDescriptor& desc, const SrcImageDescriptor& imageDesc)
{
    GLTexSubImageCubeArray(
        desc.mipLevel,
        desc.textureCube.x,
        desc.textureCube.y,
        desc.textureCube.layerOffset,
        desc.textureCube.cubeFaceOffset,
        desc.textureCube.width,
        desc.textureCube.height,
        desc.textureCube.cubeFaces,
        imageDesc
    );
}

#endif


} // /namespace LLGL



// ================================================================================
