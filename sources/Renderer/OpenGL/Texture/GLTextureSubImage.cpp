/*
 * GLTextureSubImage.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLTextureSubImage.h"
#include "../GLTypes.h"
#include "../Ext/GLExtensions.h"
#include <array>
#include <algorithm>


namespace LLGL
{


#if defined GL_ARB_direct_state_access && defined LLGL_GL_ENABLE_DSA_EXT

static void QueryGLInternalFormat(GLuint texID, GLenum& internalFormat)
{
    if (internalFormat == 0)
    {
        GLint format = 0;
        glGetNamedRenderbufferParameteriv(texID, GL_TEXTURE_INTERNAL_FORMAT, &format);
        internalFormat = static_cast<GLenum>(format);
    }
}

static void GLTextureSubImage1DBase(
    GLuint                      texID,
    std::uint32_t               mipLevel,
    std::int32_t                x,
    std::uint32_t               width,
    const SrcImageDescriptor&   imageDesc,
    GLenum                      internalFormat)
{
    QueryGLInternalFormat(texID, internalFormat);
    if (IsCompressedFormat(imageDesc.format))
    {
        glCompressedTextureSubImage1D(
            texID,
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
        glTextureSubImage1D(
            texID,
            static_cast<GLint>(mipLevel),
            x,
            static_cast<GLsizei>(width),
            GLTypes::Map(imageDesc.format, GLTypes::IsIntegerTypedFormat(internalFormat)),
            GLTypes::Map(imageDesc.dataType),
            imageDesc.data
        );
    }
}

static void GLTextureSubImage2DBase(
    GLuint                      texID,
    std::uint32_t               mipLevel,
    std::int32_t                x,
    std::int32_t                y,
    std::uint32_t               width,
    std::uint32_t               height,
    const SrcImageDescriptor&   imageDesc,
    GLenum                      internalFormat)
{
    QueryGLInternalFormat(texID, internalFormat);
    if (IsCompressedFormat(imageDesc.format))
    {
        glCompressedTextureSubImage2D(
            texID,
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
        glTextureSubImage2D(
            texID,
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

static void GLTextureSubImage3DBase(
    GLuint                      texID,
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
    QueryGLInternalFormat(texID, internalFormat);
    if (IsCompressedFormat(imageDesc.format))
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
            static_cast<GLsizei>(imageDesc.dataSize),
            imageDesc.data
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
            GLTypes::Map(imageDesc.format, GLTypes::IsIntegerTypedFormat(internalFormat)),
            GLTypes::Map(imageDesc.dataType),
            imageDesc.data
        );
    }
}

static void GLTextureSubImage1D(GLuint texID, const TextureRegion& region, const SrcImageDescriptor& imageDesc, GLenum internalFormat)
{
    GLTextureSubImage1DBase(
        texID,
        region.subresource.baseMipLevel,
        region.offset.x,
        region.extent.width,
        imageDesc,
        internalFormat
    );
}

static void GLTextureSubImage2D(GLuint texID, const TextureRegion& region, const SrcImageDescriptor& imageDesc, GLenum internalFormat)
{
    GLTextureSubImage2DBase(
        texID,
        region.subresource.baseMipLevel,
        region.offset.x,
        region.offset.y,
        region.extent.width,
        region.extent.height,
        imageDesc,
        internalFormat
    );
}

static void GLTextureSubImage3D(GLuint texID, const TextureRegion& region, const SrcImageDescriptor& imageDesc, GLenum internalFormat)
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
        imageDesc,
        internalFormat
    );
}

static void GLTextureSubImageCube(GLuint texID, const TextureRegion& region, const SrcImageDescriptor& imageDesc, GLenum internalFormat)
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
        imageDesc,
        internalFormat
    );
}

static void GLTextureSubImage1DArray(GLuint texID, const TextureRegion& region, const SrcImageDescriptor& imageDesc, GLenum internalFormat)
{
    GLTextureSubImage2DBase(
        texID,
        region.subresource.baseMipLevel,
        region.offset.x,
        region.subresource.baseArrayLayer,
        region.extent.width,
        region.subresource.numArrayLayers,
        imageDesc,
        internalFormat
    );
}

static void GLTextureSubImage2DArray(GLuint texID, const TextureRegion& region, const SrcImageDescriptor& imageDesc, GLenum internalFormat)
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
        imageDesc,
        internalFormat
    );
}

static void GLTextureSubImageCubeArray(GLuint texID, const TextureRegion& region, const SrcImageDescriptor& imageDesc, GLenum internalFormat)
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
        imageDesc,
        internalFormat
    );
}

#endif // GL_ARB_direct_state_access && LLGL_GL_ENABLE_DSA_EXT

void GLTextureSubImage(
    GLuint                      texID,
    const TextureType           type,
    const TextureRegion&        region,
    const SrcImageDescriptor&   imageDesc,
    GLenum                      internalFormat)
{
    #if defined GL_ARB_direct_state_access && defined LLGL_GL_ENABLE_DSA_EXT
    switch (type)
    {
        case TextureType::Texture1D:
            GLTextureSubImage1D(texID, region, imageDesc, internalFormat);
            break;

        case TextureType::Texture2D:
            GLTextureSubImage2D(texID, region, imageDesc, internalFormat);
            break;

        case TextureType::Texture3D:
            GLTextureSubImage3D(texID, region, imageDesc, internalFormat);
            break;

        case TextureType::TextureCube:
            GLTextureSubImageCube(texID, region, imageDesc, internalFormat);
            break;

        case TextureType::Texture1DArray:
            GLTextureSubImage1DArray(texID, region, imageDesc, internalFormat);
            break;

        case TextureType::Texture2DArray:
            GLTextureSubImage2DArray(texID, region, imageDesc, internalFormat);
            break;

        case TextureType::TextureCubeArray:
            GLTextureSubImageCubeArray(texID, region, imageDesc, internalFormat);
            break;

        default:
            break;
    }
    #endif // GL_ARB_direct_state_access && LLGL_GL_ENABLE_DSA_EXT
}


} // /namespace LLGL



// ================================================================================
