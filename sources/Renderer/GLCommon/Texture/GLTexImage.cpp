/*
 * GLTexImage.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLTexImage.h"
#include "../GLTypes.h"
#include "../GLImport.h"
#include "../GLImportExt.h"
#include "../GLExtensionRegistry.h"
#include <array>
#include <algorithm>


namespace LLGL
{


/* ----- Internal ----- */

static ImageInitialization g_imageInitialization;

void GLTexImageInitialization(const ImageInitialization& imageInitialization)
{
    g_imageInitialization = imageInitialization;
}

static std::vector<ColorRGBAf> GenImageDataRGBAf(std::uint32_t numPixels, const ColorRGBAf& color)
{
    return std::vector<ColorRGBAf>(static_cast<std::size_t>(numPixels), color);
}

static std::vector<float> GenImageDataRf(std::uint32_t numPixels, float value)
{
    return std::vector<float>(static_cast<std::size_t>(numPixels), value);
}

[[noreturn]]
static void ErrIllegalUseOfDepthFormat()
{
    throw std::runtime_error("illegal use of depth-stencil format for texture");
}

#ifdef GL_ARB_texture_storage

// Returns true if the specified GL texture target is a cube face other than GL_TEXTURE_CUBE_MAP_POSITIVE_X
static bool IsSecondaryCubeFaceTarget(GLenum target)
{
    return
    (
        target == GL_TEXTURE_CUBE_MAP_NEGATIVE_X ||
        target == GL_TEXTURE_CUBE_MAP_POSITIVE_Y ||
        target == GL_TEXTURE_CUBE_MAP_NEGATIVE_Y ||
        target == GL_TEXTURE_CUBE_MAP_POSITIVE_Z ||
        target == GL_TEXTURE_CUBE_MAP_NEGATIVE_Z
    );
}

#endif // /GL_ARB_texture_storage

/* ----- Back-end OpenGL functions ----- */

#ifdef LLGL_OPENGL

static void GLTexImage1DBase(
    GLenum          target,
    std::uint32_t   mipLevels,
    const Format    textureFormat,
    std::uint32_t   width,
    GLenum          format,
    GLenum          type,
    const void*     data,
    std::size_t     dataSize)
{
    auto internalFormat = GLTypes::Map(textureFormat);
    auto sx             = static_cast<GLsizei>(width);

    #ifdef GL_ARB_texture_storage
    if (HasExtension(GLExt::ARB_texture_storage))
    {
        /* Allocate immutable texture storage */
        glTexStorage1D(target, static_cast<GLsizei>(mipLevels), internalFormat, sx);

        /* Initialize highest MIP level */
        if (data != nullptr)
        {
            if (IsCompressedFormat(textureFormat))
                glCompressedTexSubImage1D(target, 0, 0, sx, format, static_cast<GLsizei>(dataSize), data);
            else
                glTexSubImage1D(target, 0, 0, sx, format, type, data);
        }
    }
    else
    #endif
    {
        /* Allocate mutable texture storage and initialize highest MIP level */
        if (IsCompressedFormat(textureFormat))
            glCompressedTexImage1D(target, 0, internalFormat, sx, 0, static_cast<GLsizei>(dataSize), data);
        else
            glTexImage1D(target, 0, internalFormat, sx, 0, format, type, data);

        /* Allocate mutable texture storage of MIP levels (emulate <glTexStorage1D>) */
        if (mipLevels > 1)
        {
            for (std::uint32_t i = 1; i < mipLevels; ++i)
            {
                sx = std::max(1u, sx / 2u);
                glTexImage1D(target, static_cast<GLint>(i), internalFormat, sx, 0, format, type, nullptr);
            }
        }
    }
}

#endif

static void GLTexImage2DBase(
    GLenum          target,
    std::uint32_t   mipLevels,
    const Format    textureFormat,
    std::uint32_t   width,
    std::uint32_t   height,
    GLenum          format,
    GLenum          type,
    const void*     data,
    std::size_t     dataSize)
{
    auto internalFormat = GLTypes::Map(textureFormat);
    auto sx             = static_cast<GLsizei>(width);
    auto sy             = static_cast<GLsizei>(height);

    #ifdef GL_ARB_texture_storage
    if (HasExtension(GLExt::ARB_texture_storage))
    {
        /* Allocate immutable texture storage (only once, not for ever cube face!) */
        if (target == GL_TEXTURE_CUBE_MAP_POSITIVE_X)
            glTexStorage2D(GL_TEXTURE_CUBE_MAP, static_cast<GLsizei>(mipLevels), internalFormat, sx, sy);
        else if (!IsSecondaryCubeFaceTarget(target))
            glTexStorage2D(target, static_cast<GLsizei>(mipLevels), internalFormat, sx, sy);

        /* Initialize highest MIP level */
        if (data != nullptr)
        {
            if (IsCompressedFormat(textureFormat))
                glCompressedTexSubImage2D(target, 0, 0, 0, sx, sy, format, static_cast<GLsizei>(dataSize), data);
            else
                glTexSubImage2D(target, 0, 0, 0, sx, sy, format, type, data);
        }
    }
    else
    #endif
    {
        /* Allocate mutable texture storage and initialize highest MIP level */
        if (IsCompressedFormat(textureFormat))
            glCompressedTexImage2D(target, 0, internalFormat, sx, sy, 0, static_cast<GLsizei>(dataSize), data);
        else
            glTexImage2D(target, 0, internalFormat, sx, sy, 0, format, type, data);

        /* Allocate mutable texture storage of MIP levels (emulate <glTexStorage2D>) */
        if (mipLevels > 1)
        {
            if (target == GL_TEXTURE_1D_ARRAY || target == GL_PROXY_TEXTURE_1D_ARRAY)
            {
                for (std::uint32_t i = 1; i < mipLevels; ++i)
                {
                    sx = std::max(1u, sx / 2u);
                    glTexImage2D(target, static_cast<GLint>(i), internalFormat, sx, sy, 0, format, type, nullptr);
                }
            }
            else
            {
                for (std::uint32_t i = 1; i < mipLevels; ++i)
                {
                    sx = std::max(1u, sx / 2u);
                    sy = std::max(1u, sy / 2u);
                    glTexImage2D(target, static_cast<GLint>(i), internalFormat, sx, sy, 0, format, type, nullptr);
                }
            }
        }
    }
}

static void GLTexImage3DBase(
    GLenum          target,
    std::uint32_t   mipLevels,
    const Format    textureFormat,
    std::uint32_t   width,
    std::uint32_t   height,
    std::uint32_t   depth,
    GLenum          format,
    GLenum          type,
    const void*     data,
    std::size_t     dataSize)
{
    auto internalFormat = GLTypes::Map(textureFormat);
    auto sx             = static_cast<GLsizei>(width);
    auto sy             = static_cast<GLsizei>(height);
    auto sz             = static_cast<GLsizei>(depth);

    #ifdef GL_ARB_texture_storage
    if (HasExtension(GLExt::ARB_texture_storage))
    {
        /* Allocate immutable texture storage */
        glTexStorage3D(target, static_cast<GLsizei>(mipLevels), internalFormat, sx, sy, sz);

        /* Initialize highest MIP level */
        if (data != nullptr)
        {
            if (IsCompressedFormat(textureFormat))
                glCompressedTexSubImage3D(target, 0, 0, 0, 0, sx, sy, sz, format, static_cast<GLsizei>(dataSize), data);
            else
                glTexSubImage3D(target, 0, 0, 0, 0, sx, sy, sz, format, type, data);
        }
    }
    else
    #endif
    {
        /* Allocate mutable texture storage and initialize highest MIP level */
        if (IsCompressedFormat(textureFormat))
            glCompressedTexImage3D(target, 0, internalFormat, sx, sy, sz, 0, static_cast<GLsizei>(dataSize), data);
        else
            glTexImage3D(target, 0, internalFormat, sx, sy, sz, 0, format, type, data);

        /* Allocate mutable texture storage of MIP levels (emulate <glTexStorage3D>) */
        if (mipLevels > 1)
        {
            if (target == GL_TEXTURE_3D || target == GL_PROXY_TEXTURE_3D)
            {
                for (std::uint32_t i = 1; i < mipLevels; ++i)
                {
                    sx = std::max(1u, sx / 2u);
                    sy = std::max(1u, sy / 2u);
                    sz = std::max(1u, sz / 2u);
                    glTexImage3D(target, static_cast<GLint>(i), internalFormat, sx, sy, sz, 0, format, type, nullptr);
                }
            }
            else
            {
                for (std::uint32_t i = 1; i < mipLevels; ++i)
                {
                    sx = std::max(1u, sx / 2u);
                    sy = std::max(1u, sy / 2u);
                    glTexImage3D(target, static_cast<GLint>(i), internalFormat, sx, sy, sz, 0, format, type, nullptr);
                }
            }
        }
    }
}

#ifdef LLGL_OPENGL

static void GLTexImage2DMultisampleBase(
    GLenum          target,
    std::uint32_t   samples,
    const Format    textureFormat,
    std::uint32_t   width,
    std::uint32_t   height,
    bool            fixedSamples)
{
    auto internalFormat         = GLTypes::Map(textureFormat);
    auto sampleCount            = static_cast<GLsizei>(samples);
    auto sx                     = static_cast<GLsizei>(width);
    auto sy                     = static_cast<GLsizei>(height);
    auto fixedSampleLocations   = static_cast<GLboolean>(fixedSamples ? GL_TRUE : GL_FALSE);

    #ifdef GL_ARB_texture_storage_multisample
    if (HasExtension(GLExt::ARB_texture_storage_multisample))
    {
        /* Allocate immutable texture storage */
        glTexStorage2DMultisample(target, sampleCount, internalFormat, sx, sy, fixedSampleLocations);
    }
    else
    #endif
    {
        /* Allocate mutable texture storage and initialize highest MIP level */
        glTexImage2DMultisample(target, sampleCount, internalFormat, sx, sy, fixedSampleLocations);
    }
}

static void GLTexImage3DMultisampleBase(
    GLenum          target,
    std::uint32_t   samples,
    const Format    internalFormat,
    std::uint32_t   width,
    std::uint32_t   height,
    std::uint32_t   depth,
    bool            fixedSamples)
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

#endif

/* ----- Wrapper functions ----- */

#ifdef LLGL_OPENGL

static void GLTexImage1D(
    std::uint32_t   mipLevels,
    const Format    internalFormat,
    std::uint32_t   width,
    GLenum          format,
    GLenum          type,
    const void*     data,
    std::size_t     compressedSize = 0)
{
    GLTexImage1DBase(GL_TEXTURE_1D, mipLevels, internalFormat, width, format, type, data, compressedSize);
}

#endif

static void GLTexImage2D(
    std::uint32_t   mipLevels,
    const Format    internalFormat,
    std::uint32_t   width,
    std::uint32_t   height,
    GLenum          format,
    GLenum          type,
    const void*     data,
    std::size_t     compressedSize = 0)
{
    GLTexImage2DBase(GL_TEXTURE_2D, mipLevels, internalFormat, width, height, format, type, data, compressedSize);
}

static void GLTexImage3D(
    std::uint32_t   mipLevels,
    const Format    internalFormat,
    std::uint32_t   width,
    std::uint32_t   height,
    std::uint32_t   depth,
    GLenum          format,
    GLenum          type,
    const void*     data,
    std::size_t     compressedSize = 0)
{
    GLTexImage3DBase(GL_TEXTURE_3D, mipLevels, internalFormat, width, height, depth, format, type, data, compressedSize);
}

static void GLTexImageCube(
    std::uint32_t   mipLevels,
    const Format    internalFormat,
    std::uint32_t   width,
    std::uint32_t   height,
    AxisDirection   cubeFace,
    GLenum          format,
    GLenum          type,
    const void*     data,
    std::size_t     compressedSize = 0)
{
    GLTexImage2DBase(GLTypes::Map(cubeFace), mipLevels, internalFormat, width, height, format, type, data, compressedSize);
}

#ifdef LLGL_OPENGL

static void GLTexImage1DArray(
    std::uint32_t   mipLevels,
    const Format    internalFormat,
    std::uint32_t   width,
    std::uint32_t   layers,
    GLenum          format,
    GLenum          type,
    const void*     data,
    std::size_t     compressedSize = 0)
{
    GLTexImage2DBase(GL_TEXTURE_1D_ARRAY, mipLevels, internalFormat, width, layers, format, type, data, compressedSize);
}

#endif

static void GLTexImage2DArray(
    std::uint32_t   mipLevels,
    const Format    internalFormat,
    std::uint32_t   width,
    std::uint32_t   height,
    std::uint32_t   layers,
    GLenum          format,
    GLenum          type,
    const void*     data,
    std::size_t     compressedSize = 0)
{
    GLTexImage3DBase(GL_TEXTURE_2D_ARRAY, mipLevels, internalFormat, width, height, layers, format, type, data, compressedSize);
}

#ifdef LLGL_OPENGL

static void GLTexImageCubeArray(
    std::uint32_t   mipLevels,
    const Format    internalFormat,
    std::uint32_t   width,
    std::uint32_t   height,
    std::uint32_t   layers,
    GLenum          format,
    GLenum          type,
    const void*     data,
    std::size_t     compressedSize = 0)
{
    GLTexImage3DBase(GL_TEXTURE_CUBE_MAP_ARRAY, mipLevels, internalFormat, width, height, layers*6, format, type, data, compressedSize);
}

static void GLTexImage2DMultisample(
    std::uint32_t   samples,
    const Format    internalFormat,
    std::uint32_t   width,
    std::uint32_t   height,
    bool            fixedSamples)
{
    GLTexImage2DMultisampleBase(GL_TEXTURE_2D_MULTISAMPLE, samples, internalFormat, width, height, fixedSamples);
}

static void GLTexImage2DMultisampleArray(
    std::uint32_t   samples,
    const Format    internalFormat,
    std::uint32_t   width,
    std::uint32_t   height,
    std::uint32_t   depth,
    bool            fixedSamples)
{
    GLTexImage3DMultisampleBase(GL_TEXTURE_2D_MULTISAMPLE_ARRAY, samples, internalFormat, width, height, depth, fixedSamples);
}

#endif

/* ----- Global functions ----- */

#ifdef LLGL_OPENGL

void GLTexImage1D(const TextureDescriptor& desc, const SrcImageDescriptor* imageDesc)
{
    if (imageDesc)
    {
        /* Setup texture image from descriptor */
        GLTexImage1D(
            NumMipLevels(desc),
            desc.format,
            desc.extent.width,
            GLTypes::Map(imageDesc->format),
            GLTypes::Map(imageDesc->dataType),
            imageDesc->data,
            imageDesc->dataSize
        );
    }
    else if (IsDepthStencilFormat(desc.format))
    {
        /* Throw runtime error for illegal use of depth-stencil format */
        ErrIllegalUseOfDepthFormat();
    }
    else if (IsCompressedFormat(desc.format) || !g_imageInitialization.enabled)
    {
        /* Allocate texture without initial data */
        GLTexImage1D(
            NumMipLevels(desc),
            desc.format,
            desc.extent.width,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            nullptr
        );
    }
    else
    {
        /* Initialize texture image with default color */
        auto image = GenImageDataRGBAf(desc.extent.width, g_imageInitialization.clearValue.color);
        GLTexImage1D(
            NumMipLevels(desc),
            desc.format,
            desc.extent.width,
            GL_RGBA,
            GL_FLOAT,
            image.data()
        );
    }
}

#endif

void GLTexImage2D(const TextureDescriptor& desc, const SrcImageDescriptor* imageDesc)
{
    if (imageDesc)
    {
        /* Setup texture image from descriptor */
        GLTexImage2D(
            NumMipLevels(desc),
            desc.format,
            desc.extent.width,
            desc.extent.height,
            GLTypes::Map(imageDesc->format),
            GLTypes::Map(imageDesc->dataType),
            imageDesc->data,
            imageDesc->dataSize
        );
    }
    else if (IsDepthStencilFormat(desc.format))
    {
        if (g_imageInitialization.enabled)
        {
            //TODO: add support for default initialization of stencil values
            /* Initialize depth texture image with default depth */
            auto image = GenImageDataRf(desc.extent.width * desc.extent.height, g_imageInitialization.clearValue.depth);
            GLTexImage2D(
                NumMipLevels(desc),
                desc.format,
                desc.extent.width,
                desc.extent.height,
                GL_DEPTH_COMPONENT,
                GL_FLOAT,
                image.data()
            );
        }
        else
        {
            /* Allocate depth texture image without initial data */
            GLTexImage2D(
                NumMipLevels(desc),
                desc.format,
                desc.extent.width,
                desc.extent.height,
                GL_DEPTH_COMPONENT,
                GL_FLOAT,
                nullptr
            );
        }
    }
    else if (IsCompressedFormat(desc.format) || !g_imageInitialization.enabled)
    {
        /* Allocate texture without initial data */
        GLTexImage2D(
            NumMipLevels(desc),
            desc.format,
            desc.extent.width,
            desc.extent.height,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            nullptr
        );
    }
    else
    {
        /* Initialize texture image with default color */
        auto image = GenImageDataRGBAf(desc.extent.width * desc.extent.height, g_imageInitialization.clearValue.color);
        GLTexImage2D(
            NumMipLevels(desc),
            desc.format,
            desc.extent.width,
            desc.extent.height,
            GL_RGBA,
            GL_FLOAT,
            image.data()
        );
    }
}

void GLTexImage3D(const TextureDescriptor& desc, const SrcImageDescriptor* imageDesc)
{
    if (imageDesc)
    {
        /* Setup texture image from descriptor */
        GLTexImage3D(
            NumMipLevels(desc),
            desc.format,
            desc.extent.width,
            desc.extent.height,
            desc.extent.depth,
            GLTypes::Map(imageDesc->format),
            GLTypes::Map(imageDesc->dataType),
            imageDesc->data,
            imageDesc->dataSize
        );
    }
    else if (IsDepthStencilFormat(desc.format))
    {
        /* Throw runtime error for illegal use of depth-stencil format */
        ErrIllegalUseOfDepthFormat();
    }
    else if (IsCompressedFormat(desc.format) || !g_imageInitialization.enabled)
    {
        /* Allocate texture without initial data */
        GLTexImage3D(
            NumMipLevels(desc),
            desc.format,
            desc.extent.width,
            desc.extent.height,
            desc.extent.depth,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            nullptr
        );
    }
    else
    {
        /* Initialize texture image with default color */
        auto image = GenImageDataRGBAf(desc.extent.width * desc.extent.height * desc.extent.depth, g_imageInitialization.clearValue.color);
        GLTexImage3D(
            NumMipLevels(desc),
            desc.format,
            desc.extent.width,
            desc.extent.height,
            desc.extent.depth,
            GL_RGBA,
            GL_FLOAT,
            image.data()
        );
    }
}

void GLTexImageCube(const TextureDescriptor& desc, const SrcImageDescriptor* imageDesc)
{
    const std::array<AxisDirection, 6> cubeFaces
    {{
        AxisDirection::XPos,
        AxisDirection::XNeg,
        AxisDirection::YPos,
        AxisDirection::YNeg,
        AxisDirection::ZPos,
        AxisDirection::ZNeg
    }};

    auto numMipLevels = NumMipLevels(desc);

    if (imageDesc)
    {
        /* Setup texture image cube-faces from descriptor */
        auto imageFace          = reinterpret_cast<const char*>(imageDesc->data);
        auto imageFaceStride    = (desc.extent.width * desc.extent.height * ImageFormatSize(imageDesc->format) * DataTypeSize(imageDesc->dataType));

        if (IsCompressedFormat(desc.format))
            imageFaceStride = static_cast<std::uint32_t>(imageDesc->dataSize);

        auto dataFormatGL       = GLTypes::Map(imageDesc->format);
        auto dataTypeGL         = GLTypes::Map(imageDesc->dataType);

        for (auto face : cubeFaces)
        {
            GLTexImageCube(
                numMipLevels,
                desc.format,
                desc.extent.width,
                desc.extent.height,
                face,
                dataFormatGL,
                dataTypeGL,
                imageFace,
                imageDesc->dataSize
            );
            imageFace += imageFaceStride;
        }
    }
    else if (IsDepthStencilFormat(desc.format))
    {
        /* Throw runtime error for illegal use of depth-stencil format */
        ErrIllegalUseOfDepthFormat();
    }
    else if (IsCompressedFormat(desc.format) || !g_imageInitialization.enabled)
    {
        /* Allocate texture without initial data */
        for (auto face : cubeFaces)
        {
            GLTexImageCube(
                numMipLevels,
                desc.format,
                desc.extent.width,
                desc.extent.height,
                face,
                GL_RGBA,
                GL_UNSIGNED_BYTE,
                nullptr
            );
        }
    }
    else
    {
        /* Initialize texture image cube-faces with default color */
        auto image = GenImageDataRGBAf(desc.extent.width * desc.extent.height, g_imageInitialization.clearValue.color);
        for (auto face : cubeFaces)
        {
            GLTexImageCube(
                numMipLevels,
                desc.format,
                desc.extent.width,
                desc.extent.height,
                face,
                GL_RGBA,
                GL_FLOAT,
                image.data()
            );
        }
    }
}

#ifdef LLGL_OPENGL

void GLTexImage1DArray(const TextureDescriptor& desc, const SrcImageDescriptor* imageDesc)
{
    if (imageDesc)
    {
        /* Setup texture image from descriptor */
        GLTexImage1DArray(
            NumMipLevels(desc),
            desc.format,
            desc.extent.width,
            desc.arrayLayers,
            GLTypes::Map(imageDesc->format),
            GLTypes::Map(imageDesc->dataType),
            imageDesc->data,
            imageDesc->dataSize
        );
    }
    else if (IsDepthStencilFormat(desc.format))
    {
        /* Throw runtime error for illegal use of depth-stencil format */
        ErrIllegalUseOfDepthFormat();
    }
    else if (IsCompressedFormat(desc.format) || !g_imageInitialization.enabled)
    {
        /* Allocate texture without initial data */
        GLTexImage1DArray(
            NumMipLevels(desc),
            desc.format,
            desc.extent.width,
            desc.arrayLayers,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            nullptr
        );
    }
    else
    {
        /* Initialize texture image with default color */
        auto image = GenImageDataRGBAf(desc.extent.width * desc.arrayLayers, g_imageInitialization.clearValue.color);
        GLTexImage1DArray(
            NumMipLevels(desc),
            desc.format,
            desc.extent.width,
            desc.arrayLayers,
            GL_RGBA,
            GL_FLOAT,
            image.data()
        );
    }
}

#endif

void GLTexImage2DArray(const TextureDescriptor& desc, const SrcImageDescriptor* imageDesc)
{
    if (imageDesc)
    {
        /* Setup texture image from descriptor */
        GLTexImage2DArray(
            NumMipLevels(desc),
            desc.format,
            desc.extent.width,
            desc.extent.height,
            desc.arrayLayers,
            GLTypes::Map(imageDesc->format),
            GLTypes::Map(imageDesc->dataType),
            imageDesc->data,
            imageDesc->dataSize
        );
    }
    else if (IsDepthStencilFormat(desc.format))
    {
        if (g_imageInitialization.enabled)
        {
            /* Initialize depth texture image with default depth */
            auto image = GenImageDataRf(desc.extent.width * desc.extent.height * desc.arrayLayers, g_imageInitialization.clearValue.depth);
            GLTexImage2DArray(
                NumMipLevels(desc),
                desc.format,
                desc.extent.width,
                desc.extent.height,
                desc.arrayLayers,
                GL_DEPTH_COMPONENT,
                GL_FLOAT,
                image.data()
            );
        }
        else
        {
            /* Allocate depth texture image without initial data */
            GLTexImage2DArray(
                NumMipLevels(desc),
                desc.format,
                desc.extent.width,
                desc.extent.height,
                desc.arrayLayers,
                GL_DEPTH_COMPONENT,
                GL_FLOAT,
                nullptr
            );
        }
    }
    else if (IsCompressedFormat(desc.format) || !g_imageInitialization.enabled)
    {
        /* Allocate texture without initial data */
        GLTexImage2DArray(
            NumMipLevels(desc),
            desc.format,
            desc.extent.width,
            desc.extent.height,
            desc.arrayLayers,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            nullptr
        );
    }
    else
    {
        /* Initialize texture image with default color */
        auto image = GenImageDataRGBAf(desc.extent.width * desc.extent.height * desc.arrayLayers, g_imageInitialization.clearValue.color);
        GLTexImage2DArray(
            NumMipLevels(desc),
            desc.format,
            desc.extent.width,
            desc.extent.height,
            desc.arrayLayers,
            GL_RGBA,
            GL_FLOAT,
            image.data()
        );
    }
}

#ifdef LLGL_OPENGL

void GLTexImageCubeArray(const TextureDescriptor& desc, const SrcImageDescriptor* imageDesc)
{
    if (imageDesc)
    {
        /* Setup texture image cube-faces from descriptor */
        GLTexImageCubeArray(
            NumMipLevels(desc),
            desc.format,
            desc.extent.width,
            desc.extent.height,
            desc.arrayLayers,
            GLTypes::Map(imageDesc->format),
            GLTypes::Map(imageDesc->dataType),
            imageDesc->data,
            imageDesc->dataSize
        );
    }
    else if (IsDepthStencilFormat(desc.format))
    {
        /* Throw runtime error for illegal use of depth-stencil format */
        ErrIllegalUseOfDepthFormat();
    }
    else if (IsCompressedFormat(desc.format) || !g_imageInitialization.enabled)
    {
        /* Allocate texture without initial data */
        GLTexImageCubeArray(
            NumMipLevels(desc),
            desc.format,
            desc.extent.width,
            desc.extent.height,
            desc.arrayLayers,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            nullptr
        );
    }
    else
    {
        /* Initialize texture image cube-faces with default color */
        auto image = GenImageDataRGBAf(desc.extent.width * desc.extent.height * desc.arrayLayers * 6u, g_imageInitialization.clearValue.color);
        GLTexImageCubeArray(
            NumMipLevels(desc),
            desc.format,
            desc.extent.width,
            desc.extent.height,
            desc.arrayLayers,
            GL_RGBA,
            GL_FLOAT,
            image.data()
        );
    }
}

void GLTexImage2DMS(const TextureDescriptor& desc)
{
    /* Setup multi-sampled texture storage from descriptor */
    GLTexImage2DMultisample(
        desc.samples,
        desc.format,
        desc.extent.width,
        desc.extent.height,
        ((desc.flags & TextureFlags::FixedSamples) != 0)
    );
}

void GLTexImage2DMSArray(const TextureDescriptor& desc)
{
    /* Setup multi-sampled array texture storage from descriptor */
    GLTexImage2DMultisampleArray(
        desc.samples,
        desc.format,
        desc.extent.width,
        desc.extent.height,
        desc.arrayLayers,
        ((desc.flags & TextureFlags::FixedSamples) != 0)
    );
}

#endif


} // /namespace LLGL



// ================================================================================
