/*
 * GLTexImage.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "GLTexImage.h"
#include "../GLTypes.h"
#include "../Profile/GLProfile.h"
#include "../Ext/GLExtensions.h"
#include "../Ext/GLExtensionRegistry.h"
#include "../../../Core/Exception.h"
#include <LLGL/Utils/ColorRGBA.h>
#include <LLGL/Utils/ForRange.h>
#include <algorithm>


namespace LLGL
{


// Generates an image buffer with floating-points for RGBA components.
static std::vector<ColorRGBAf> GenImageDataRGBAf(std::uint32_t numPixels, const float (&color)[4])
{
    return std::vector<ColorRGBAf>(static_cast<std::size_t>(numPixels), ColorRGBAf{ color[0], color[1], color[2], color[3] });
}

// Generates an image buffer with floating-points for the Red component.
static std::vector<float> GenImageDataRf(std::uint32_t numPixels, float value)
{
    return std::vector<float>(static_cast<std::size_t>(numPixels), value);
}

// Generates an image buffer with floating-points for the Red component and unsigned bytes for the Green component.
static std::vector<GLDepthStencilPair> GenImageDataD32fS8ui(std::uint32_t numPixels, float depth, std::uint8_t stencil)
{
    return std::vector<GLDepthStencilPair>(static_cast<std::size_t>(numPixels), GLDepthStencilPair{ depth, stencil });
}

// Generates an image buffer with unsigned bytes for the stencil index.
static std::vector<std::uint8_t> GenImageDataS8ui(std::uint32_t numPixels, std::uint8_t stencil)
{
    return std::vector<std::uint8_t>(static_cast<std::size_t>(numPixels), stencil);
}

// Returns true if the specified hardware format requires an integer type, e.g. GL_RGBA_INTEGER
static bool IsStrictFloatFormat(const Format format)
{
    return (IsFloatFormat(format) && !IsNormalizedFormat(format));
}

// Returns true if the 'clearValue' member is enabled if no initial image data is specified, i.e. MiscFlags::NoInitialData is NOT specified
static bool IsClearValueEnabled(const TextureDescriptor& desc)
{
    return ((desc.miscFlags & MiscFlags::NoInitialData) == 0);
}

// Returns true if a GL texture with the specified descriptor can be default initialized with an RGBA float format, i.e. GL_RGBA and GL_FLOAT.
static bool CanInitializeTexWithRGBAf(const TextureDescriptor& desc)
{
    return (IsClearValueEnabled(desc) && !IsCompressedFormat(desc.format) && IsStrictFloatFormat(desc.format));
}

static GLenum GetDefaultInitialGLImageFormat(Format format)
{
    #if !LLGL_GL_ENABLE_OPENGL2X
    return (IsIntegerFormat(format) ? GL_RGBA_INTEGER : GL_RGBA);
    #else
    return GL_RGBA;
    #endif
}

[[noreturn]]
static void ErrIllegalUseOfDepthFormat()
{
    LLGL_TRAP("illegal use of depth-stencil format for texture");
}

// Converts the internal format if necessary
static Format FindSuitableDepthFormat(const TextureDescriptor& desc)
{
    if (IsDepthOrStencilFormat(desc.format))
    {
        if ((desc.bindFlags & BindFlags::ColorAttachment) != 0)
        {
            /* Depth-stencil formats that are used as color attachments must be converted to a color renderable format */
            switch (desc.format)
            {
                case Format::D16UNorm:  return Format::R16UNorm;
                case Format::D32Float:  return Format::R32Float;
                default:                break;
            }
        }
    }
    return desc.format;
}

#if LLGL_GLEXT_TEXTURE_STORAGE

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

#endif // /LLGL_GLEXT_TEXTURE_STORAGE

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
    GLenum  internalFormat  = GLTypes::Map(textureFormat);
    GLsizei sx              = static_cast<GLsizei>(width);

    #if LLGL_GLEXT_TEXTURE_STORAGE
    if (HasExtension(GLExt::ARB_texture_storage))
    {
        /* Allocate immutable texture storage */
        glTexStorage1D(target, static_cast<GLsizei>(mipLevels), internalFormat, sx);

        /* Initialize highest MIP level */
        if (data != nullptr)
        {
            /* Use <internalFormat> for the compressed version, and <format> for the uncompressed version */
            if (IsCompressedFormat(textureFormat))
                glCompressedTexSubImage1D(target, 0, 0, sx, internalFormat, static_cast<GLsizei>(dataSize), data);
            else
                glTexSubImage1D(target, 0, 0, sx, format, type, data);
        }
    }
    else
    #endif // /LLGL_GLEXT_TEXTURE_STORAGE
    {
        /* Allocate mutable texture storage and initialize highest MIP level */
        if (IsCompressedFormat(textureFormat))
            glCompressedTexImage1D(target, 0, internalFormat, sx, 0, static_cast<GLsizei>(dataSize), data);
        else
            glTexImage1D(target, 0, internalFormat, sx, 0, format, type, data);

        /* Allocate mutable texture storage of MIP levels (emulate <glTexStorage1D>) */
        if (mipLevels > 1)
        {
            for_subrange(mipLevel, 1, mipLevels)
            {
                sx = std::max(1u, sx / 2u);
                glTexImage1D(target, static_cast<GLint>(mipLevel), internalFormat, sx, 0, format, type, nullptr);
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
    GLenum  internalFormat  = GLTypes::Map(textureFormat);
    GLsizei sx              = static_cast<GLsizei>(width);
    GLsizei sy              = static_cast<GLsizei>(height);

    #if LLGL_GLEXT_TEXTURE_STORAGE
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
            /* Use <internalFormat> for the compressed version, and <format> for the uncompressed version */
            if (IsCompressedFormat(textureFormat))
                glCompressedTexSubImage2D(target, 0, 0, 0, sx, sy, internalFormat, static_cast<GLsizei>(dataSize), data);
            else
                glTexSubImage2D(target, 0, 0, 0, sx, sy, format, type, data);
        }
    }
    else
    #endif // /LLGL_GLEXT_TEXTURE_STORAGE
    {
        /* Allocate mutable texture storage and initialize highest MIP level */
        if (IsCompressedFormat(textureFormat))
            glCompressedTexImage2D(target, 0, internalFormat, sx, sy, 0, static_cast<GLsizei>(dataSize), data);
        else
            glTexImage2D(target, 0, internalFormat, sx, sy, 0, format, type, data);

        /* Allocate mutable texture storage of MIP levels (emulate <glTexStorage2D>) */
        if (mipLevels > 1)
        {
            #ifdef LLGL_OPENGL
            if (target == GL_TEXTURE_1D_ARRAY || target == GL_PROXY_TEXTURE_1D_ARRAY)
            {
                for_subrange(mipLevel, 1, mipLevels)
                {
                    sx = std::max(1u, sx / 2u);
                    glTexImage2D(target, static_cast<GLint>(mipLevel), internalFormat, sx, sy, 0, format, type, nullptr);
                }
            }
            else
            #endif
            {
                for_subrange(mipLevel, 1, mipLevels)
                {
                    sx = std::max(1u, sx / 2u);
                    sy = std::max(1u, sy / 2u);
                    glTexImage2D(target, static_cast<GLint>(mipLevel), internalFormat, sx, sy, 0, format, type, nullptr);
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
    GLenum  internalFormat  = GLTypes::Map(textureFormat);
    GLsizei sx              = static_cast<GLsizei>(width);
    GLsizei sy              = static_cast<GLsizei>(height);
    GLsizei sz              = static_cast<GLsizei>(depth);

    #if LLGL_GLEXT_TEXTURE_STORAGE
    if (HasExtension(GLExt::ARB_texture_storage))
    {
        /* Allocate immutable texture storage */
        glTexStorage3D(target, static_cast<GLsizei>(mipLevels), internalFormat, sx, sy, sz);

        /* Initialize highest MIP level */
        if (data != nullptr)
        {
            /* Use <internalFormat> for the compressed version, and <format> for the uncompressed version */
            if (IsCompressedFormat(textureFormat))
                glCompressedTexSubImage3D(target, 0, 0, 0, 0, sx, sy, sz, internalFormat, static_cast<GLsizei>(dataSize), data);
            else
                glTexSubImage3D(target, 0, 0, 0, 0, sx, sy, sz, format, type, data);
        }
    }
    else
    #endif // /LLGL_GLEXT_TEXTURE_STORAGE
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
                for_subrange(mipLevel, 1, mipLevels)
                {
                    sx = std::max(1u, sx / 2u);
                    sy = std::max(1u, sy / 2u);
                    sz = std::max(1u, sz / 2u);
                    glTexImage3D(target, static_cast<GLint>(mipLevel), internalFormat, sx, sy, sz, 0, format, type, nullptr);
                }
            }
            else
            {
                for_subrange(mipLevel, 1, mipLevels)
                {
                    sx = std::max(1u, sx / 2u);
                    sy = std::max(1u, sy / 2u);
                    glTexImage3D(target, static_cast<GLint>(mipLevel), internalFormat, sx, sy, sz, 0, format, type, nullptr);
                }
            }
        }
    }
}

#if LLGL_GLEXT_TEXTURE_MULTISAMPLE

static void GLTexImage2DMultisampleBase(
    GLenum          target,
    std::uint32_t   samples,
    const Format    textureFormat,
    std::uint32_t   width,
    std::uint32_t   height,
    bool            fixedSamples)
{
    GLenum      internalFormat          = GLTypes::Map(textureFormat);
    GLsizei     sampleCount             = static_cast<GLsizei>(samples);
    GLsizei     sx                      = static_cast<GLsizei>(width);
    GLsizei     sy                      = static_cast<GLsizei>(height);
    GLboolean   fixedSampleLocations    = static_cast<GLboolean>(fixedSamples ? GL_TRUE : GL_FALSE);

    #if LLGL_GLEXT_TEXTURE_STORAGE_MULTISAMPLE
    if (HasExtension(GLExt::ARB_texture_storage_multisample))
    {
        /* Allocate immutable texture storage */
        glTexStorage2DMultisample(target, sampleCount, internalFormat, sx, sy, fixedSampleLocations);
    }
    else
    #endif
    {
        /* Allocate mutable texture storage */
        glTexImage2DMultisample(target, sampleCount, internalFormat, sx, sy, fixedSampleLocations);
    }
}

static void GLTexImage3DMultisampleBase(
    GLenum          target,
    std::uint32_t   samples,
    const Format    textureFormat,
    std::uint32_t   width,
    std::uint32_t   height,
    std::uint32_t   depth,
    bool            fixedSamples)
{
    GLenum      internalFormat          = GLTypes::Map(textureFormat);
    GLsizei     sampleCount             = static_cast<GLsizei>(samples);
    GLsizei     sx                      = static_cast<GLsizei>(width);
    GLsizei     sy                      = static_cast<GLsizei>(height);
    GLsizei     sz                      = static_cast<GLsizei>(depth);
    GLboolean   fixedSampleLocations    = static_cast<GLboolean>(fixedSamples ? GL_TRUE : GL_FALSE);

    #if LLGL_GLEXT_TEXTURE_STORAGE_MULTISAMPLE
    if (HasExtension(GLExt::ARB_texture_storage_multisample))
    {
        /* Allocate immutable texture storage */
        glTexStorage3DMultisample(target, sampleCount, internalFormat, sx, sy, sz, fixedSampleLocations);
    }
    else
    #endif
    {
        /* Allocate mutable texture storage */
        glTexImage3DMultisample(target, sampleCount, internalFormat, sx, sy, sz, fixedSampleLocations);
    }
}

#endif // /LLGL_GLEXT_TEXTURE_MULTISAMPLE

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
    std::uint32_t   cubeFaceIndex,
    GLenum          format,
    GLenum          type,
    const void*     data,
    std::size_t     compressedSize = 0)
{
    GLTexImage2DBase(GLTypes::ToTextureCubeMap(cubeFaceIndex), mipLevels, internalFormat, width, height, format, type, data, compressedSize);
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

#if LLGL_GLEXT_TEXTURE_MULTISAMPLE

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
    GLTexImage3DBase(GL_TEXTURE_CUBE_MAP_ARRAY, mipLevels, internalFormat, width, height, layers, format, type, data, compressedSize);
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

#endif // /LLGL_GLEXT_TEXTURE_MULTISAMPLE

#ifdef LLGL_OPENGL

static void GLTexImage1D(const TextureDescriptor& desc, const ImageView* imageView)
{
    if (imageView != nullptr)
    {
        /* Setup texture image from descriptor */
        GLTexImage1D(
            NumMipLevels(desc),
            desc.format,
            desc.extent.width,
            GLTypes::Map(imageView->format, IsIntegerFormat(desc.format)),
            GLTypes::Map(imageView->dataType),
            imageView->data,
            imageView->dataSize
        );
    }
    else if (IsDepthOrStencilFormat(desc.format))
    {
        /* Throw runtime error for illegal use of depth-stencil format */
        ErrIllegalUseOfDepthFormat();
    }
    else if (CanInitializeTexWithRGBAf(desc))
    {
        /* Initialize texture image with default color */
        auto image = GenImageDataRGBAf(desc.extent.width, desc.clearValue.color);
        GLTexImage1D(
            NumMipLevels(desc),
            desc.format,
            desc.extent.width,
            GL_RGBA,
            GL_FLOAT,
            image.data()
        );
    }
    else
    {
        /* Allocate texture without initial data */
        GLTexImage1D(
            NumMipLevels(desc),
            desc.format,
            desc.extent.width,
            GetDefaultInitialGLImageFormat(desc.format),
            GL_UNSIGNED_BYTE,
            nullptr
        );
    }
}

#endif

static void GLTexImage2D(const TextureDescriptor& desc, const ImageView* imageView)
{
    if (imageView != nullptr)
    {
        /* Setup texture image from descriptor */
        GLTexImage2D(
            NumMipLevels(desc),
            desc.format,
            desc.extent.width,
            desc.extent.height,
            GLTypes::Map(imageView->format, IsIntegerFormat(desc.format)),
            GLTypes::Map(imageView->dataType),
            imageView->data,
            imageView->dataSize
        );
    }
    else if (IsStencilFormat(desc.format))
    {
        #if !LLGL_GL_ENABLE_OPENGL2X
        if (IsClearValueEnabled(desc))
        {
            /* Initialize depth-stencil texture image with default depth */
            auto image = GenImageDataD32fS8ui(desc.extent.width * desc.extent.height, desc.clearValue.depth, desc.clearValue.stencil);
            GLTexImage2D(
                NumMipLevels(desc),
                FindSuitableDepthFormat(desc),
                desc.extent.width,
                desc.extent.height,
                GL_DEPTH_STENCIL,
                GL_FLOAT_32_UNSIGNED_INT_24_8_REV,
                image.data()
            );
        }
        else
        {
            /* Allocate depth-stencil texture image without initial data */
            GLTexImage2D(
                NumMipLevels(desc),
                FindSuitableDepthFormat(desc),
                desc.extent.width,
                desc.extent.height,
                GL_DEPTH_STENCIL,
                GL_FLOAT_32_UNSIGNED_INT_24_8_REV,
                nullptr
            );
        }
        #else
        LLGL_TRAP_FEATURE_NOT_SUPPORTED("stencil-index texture format");
        #endif
    }
    else if (IsDepthFormat(desc.format))
    {
        if (IsClearValueEnabled(desc))
        {
            /* Initialize depth texture image with default depth */
            auto image = GenImageDataRf(desc.extent.width * desc.extent.height, desc.clearValue.depth);
            GLTexImage2D(
                NumMipLevels(desc),
                FindSuitableDepthFormat(desc),
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
                FindSuitableDepthFormat(desc),
                desc.extent.width,
                desc.extent.height,
                GL_DEPTH_COMPONENT,
                GL_FLOAT,
                nullptr
            );
        }
    }
    else if (CanInitializeTexWithRGBAf(desc))
    {
        /* Initialize texture image with default color */
        auto image = GenImageDataRGBAf(desc.extent.width * desc.extent.height, desc.clearValue.color);
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
    else
    {
        /* Allocate texture without initial data */
        GLTexImage2D(
            NumMipLevels(desc),
            desc.format,
            desc.extent.width,
            desc.extent.height,
            GetDefaultInitialGLImageFormat(desc.format),
            GL_UNSIGNED_BYTE,
            nullptr
        );
    }
}

static void GLTexImage3D(const TextureDescriptor& desc, const ImageView* imageView)
{
    if (imageView != nullptr)
    {
        /* Setup texture image from descriptor */
        GLTexImage3D(
            NumMipLevels(desc),
            desc.format,
            desc.extent.width,
            desc.extent.height,
            desc.extent.depth,
            GLTypes::Map(imageView->format, IsIntegerFormat(desc.format)),
            GLTypes::Map(imageView->dataType),
            imageView->data,
            imageView->dataSize
        );
    }
    else if (IsDepthOrStencilFormat(desc.format))
    {
        /* Throw runtime error for illegal use of depth-stencil format */
        ErrIllegalUseOfDepthFormat();
    }
    else if (CanInitializeTexWithRGBAf(desc))
    {
        /* Initialize texture image with default color */
        auto image = GenImageDataRGBAf(desc.extent.width * desc.extent.height * desc.extent.depth, desc.clearValue.color);
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
    else
    {
        /* Allocate texture without initial data */
        GLTexImage3D(
            NumMipLevels(desc),
            desc.format,
            desc.extent.width,
            desc.extent.height,
            desc.extent.depth,
            GetDefaultInitialGLImageFormat(desc.format),
            GL_UNSIGNED_BYTE,
            nullptr
        );
    }
}

static void GLTexImageCube(const TextureDescriptor& desc, const ImageView* imageView)
{
    const std::uint32_t numMipLevels = NumMipLevels(desc);

    if (imageView != nullptr)
    {
        /* Setup texture image cube-faces from descriptor */
        const char* imageFace       = reinterpret_cast<const char*>(imageView->data);
        std::size_t imageFaceStride = GetMemoryFootprint(imageView->format, imageView->dataType, desc.extent.width * desc.extent.height);

        if (IsCompressedFormat(desc.format))
            imageFaceStride = imageView->dataSize;

        GLenum dataFormatGL       = GLTypes::Map(imageView->format, IsIntegerFormat(desc.format));
        GLenum dataTypeGL         = GLTypes::Map(imageView->dataType);

        for_range(arrayLayer, desc.arrayLayers)
        {
            GLTexImageCube(
                numMipLevels,
                desc.format,
                desc.extent.width,
                desc.extent.height,
                arrayLayer,
                dataFormatGL,
                dataTypeGL,
                imageFace,
                imageView->dataSize
            );
            imageFace += imageFaceStride;
        }
    }
    else if (IsStencilFormat(desc.format))
    {
        Format internalFormat = FindSuitableDepthFormat(desc);

        #if !LLGL_GL_ENABLE_OPENGL2X
        std::vector<GLDepthStencilPair> image;
        const void* initialData = nullptr;

        if (IsClearValueEnabled(desc))
        {
            /* Initialize depth texture image with default depth */
            image       = GenImageDataD32fS8ui(desc.extent.width * desc.extent.height, desc.clearValue.depth, desc.clearValue.stencil);
            initialData = image.data();
        }

        /* Allocate depth texture image without initial data */
        for_range(arrayLayer, desc.arrayLayers)
        {
            GLTexImageCube(
                numMipLevels,
                internalFormat,
                desc.extent.width,
                desc.extent.height,
                arrayLayer,
                GL_DEPTH_STENCIL,
                GL_FLOAT_32_UNSIGNED_INT_24_8_REV,
                initialData
            );
        }
        #else
        LLGL_TRAP_FEATURE_NOT_SUPPORTED("stencil-index texture format");
        #endif
    }
    else if (IsDepthFormat(desc.format))
    {
        Format internalFormat = FindSuitableDepthFormat(desc);

        std::vector<float> image;
        const void* initialData = nullptr;

        if (IsClearValueEnabled(desc))
        {
            /* Initialize depth texture image with default depth */
            image       = GenImageDataRf(desc.extent.width * desc.extent.height, desc.clearValue.depth);
            initialData = image.data();
        }

        /* Allocate depth texture image without initial data */
        for_range(arrayLayer, desc.arrayLayers)
        {
            GLTexImageCube(
                numMipLevels,
                internalFormat,
                desc.extent.width,
                desc.extent.height,
                arrayLayer,
                GL_DEPTH_COMPONENT,
                GL_FLOAT,
                initialData
            );
        }
    }
    else if (CanInitializeTexWithRGBAf(desc))
    {
        /* Initialize texture image cube-faces with default color */
        auto image = GenImageDataRGBAf(desc.extent.width * desc.extent.height, desc.clearValue.color);
        for_range(arrayLayer, desc.arrayLayers)
        {
            GLTexImageCube(
                numMipLevels,
                desc.format,
                desc.extent.width,
                desc.extent.height,
                arrayLayer,
                GL_RGBA,
                GL_FLOAT,
                image.data()
            );
        }
    }
    else
    {
        /* Allocate texture without initial data */
        for_range(arrayLayer, desc.arrayLayers)
        {
            GLTexImageCube(
                numMipLevels,
                desc.format,
                desc.extent.width,
                desc.extent.height,
                arrayLayer,
                GetDefaultInitialGLImageFormat(desc.format),
                GL_UNSIGNED_BYTE,
                nullptr
            );
        }
    }
}

#ifdef LLGL_OPENGL

static void GLTexImage1DArray(const TextureDescriptor& desc, const ImageView* imageView)
{
    if (imageView != nullptr)
    {
        /* Setup texture image from descriptor */
        GLTexImage1DArray(
            NumMipLevels(desc),
            desc.format,
            desc.extent.width,
            desc.arrayLayers,
            GLTypes::Map(imageView->format, IsIntegerFormat(desc.format)),
            GLTypes::Map(imageView->dataType),
            imageView->data,
            imageView->dataSize
        );
    }
    else if (IsDepthOrStencilFormat(desc.format))
    {
        /* Throw runtime error for illegal use of depth-stencil format */
        ErrIllegalUseOfDepthFormat();
    }
    else if (CanInitializeTexWithRGBAf(desc))
    {
        /* Initialize texture image with default color */
        auto image = GenImageDataRGBAf(desc.extent.width * desc.arrayLayers, desc.clearValue.color);
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
    else
    {
        /* Allocate texture without initial data */
        GLTexImage1DArray(
            NumMipLevels(desc),
            desc.format,
            desc.extent.width,
            desc.arrayLayers,
            GetDefaultInitialGLImageFormat(desc.format),
            GL_UNSIGNED_BYTE,
            nullptr
        );
    }
}

#endif

static void GLTexImage2DArray(const TextureDescriptor& desc, const ImageView* imageView)
{
    if (imageView != nullptr)
    {
        /* Setup texture image from descriptor */
        GLTexImage2DArray(
            NumMipLevels(desc),
            desc.format,
            desc.extent.width,
            desc.extent.height,
            desc.arrayLayers,
            GLTypes::Map(imageView->format, IsIntegerFormat(desc.format)),
            GLTypes::Map(imageView->dataType),
            imageView->data,
            imageView->dataSize
        );
    }
    else if (IsStencilFormat(desc.format))
    {
        #if !LLGL_GL_ENABLE_OPENGL2X
        if (IsClearValueEnabled(desc))
        {
            /* Initialize depth-stencil texture image with default depth */
            auto image = GenImageDataD32fS8ui(desc.extent.width * desc.extent.height * desc.arrayLayers, desc.clearValue.depth, desc.clearValue.stencil);
            GLTexImage2DArray(
                NumMipLevels(desc),
                FindSuitableDepthFormat(desc),
                desc.extent.width,
                desc.extent.height,
                desc.arrayLayers,
                GL_DEPTH_STENCIL,
                GL_FLOAT_32_UNSIGNED_INT_24_8_REV,
                image.data()
            );
        }
        else
        {
            /* Allocate depth-stencil texture image without initial data */
            GLTexImage2DArray(
                NumMipLevels(desc),
                FindSuitableDepthFormat(desc),
                desc.extent.width,
                desc.extent.height,
                desc.arrayLayers,
                GL_DEPTH_STENCIL,
                GL_FLOAT_32_UNSIGNED_INT_24_8_REV,
                nullptr
            );
        }
        #else
        LLGL_TRAP_FEATURE_NOT_SUPPORTED("stencil-index texture format");
        #endif
    }
    else if (IsDepthFormat(desc.format))
    {
        if (IsClearValueEnabled(desc))
        {
            /* Initialize depth texture image with default depth */
            auto image = GenImageDataRf(desc.extent.width * desc.extent.height * desc.arrayLayers, desc.clearValue.depth);
            GLTexImage2DArray(
                NumMipLevels(desc),
                FindSuitableDepthFormat(desc),
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
                FindSuitableDepthFormat(desc),
                desc.extent.width,
                desc.extent.height,
                desc.arrayLayers,
                GL_DEPTH_COMPONENT,
                GL_FLOAT,
                nullptr
            );
        }
    }
    else if (CanInitializeTexWithRGBAf(desc))
    {
        /* Initialize texture image with default color */
        auto image = GenImageDataRGBAf(desc.extent.width * desc.extent.height * desc.arrayLayers, desc.clearValue.color);
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
    else
    {
        /* Allocate texture without initial data */
        GLTexImage2DArray(
            NumMipLevels(desc),
            desc.format,
            desc.extent.width,
            desc.extent.height,
            desc.arrayLayers,
            GetDefaultInitialGLImageFormat(desc.format),
            GL_UNSIGNED_BYTE,
            nullptr
        );
    }
}

#if LLGL_GLEXT_TEXTURE_MULTISAMPLE

static void GLTexImageCubeArray(const TextureDescriptor& desc, const ImageView* imageView)
{
    if (imageView != nullptr)
    {
        /* Setup texture image cube-faces from descriptor */
        GLTexImageCubeArray(
            NumMipLevels(desc),
            desc.format,
            desc.extent.width,
            desc.extent.height,
            desc.arrayLayers,
            GLTypes::Map(imageView->format, IsIntegerFormat(desc.format)),
            GLTypes::Map(imageView->dataType),
            imageView->data,
            imageView->dataSize
        );
    }
    else if (IsDepthOrStencilFormat(desc.format))
    {
        /* Throw runtime error for illegal use of depth-stencil format */
        ErrIllegalUseOfDepthFormat();
    }
    else if (CanInitializeTexWithRGBAf(desc))
    {
        /* Initialize texture image cube-faces with default color */
        auto image = GenImageDataRGBAf(desc.extent.width * desc.extent.height * desc.arrayLayers, desc.clearValue.color);
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
    else
    {
        /* Allocate texture without initial data */
        GLTexImageCubeArray(
            NumMipLevels(desc),
            desc.format,
            desc.extent.width,
            desc.extent.height,
            desc.arrayLayers,
            GetDefaultInitialGLImageFormat(desc.format),
            GL_UNSIGNED_BYTE,
            nullptr
        );
    }
}

static void GLTexImage2DMS(const TextureDescriptor& desc)
{
    /* Setup multi-sampled texture storage from descriptor */
    GLTexImage2DMultisample(
        desc.samples,
        desc.format,
        desc.extent.width,
        desc.extent.height,
        ((desc.miscFlags & MiscFlags::FixedSamples) != 0)
    );
}

static void GLTexImage2DMSArray(const TextureDescriptor& desc)
{
    /* Setup multi-sampled array texture storage from descriptor */
    GLTexImage2DMultisampleArray(
        desc.samples,
        desc.format,
        desc.extent.width,
        desc.extent.height,
        desc.arrayLayers,
        ((desc.miscFlags & MiscFlags::FixedSamples) != 0)
    );
}

#endif // /LLGL_GLEXT_TEXTURE_MULTISAMPLE

bool GLTexImage(const TextureDescriptor& desc, const ImageView* imageView)
{
    //TODO: on-the-fly decompression would be awesome (if GL_ARB_texture_compression is unsupported), but a lot of work :-/
    /* If compressed format is requested, GL_ARB_texture_compression must be supported */
    if (IsCompressedFormat(desc.format) && !HasExtension(GLExt::ARB_texture_compression))
        return false;

    switch (desc.type)
    {
        #ifdef LLGL_OPENGL
        case TextureType::Texture1D:
            GLTexImage1D(desc, imageView);
            break;
        #endif

        case TextureType::Texture2D:
            GLTexImage2D(desc, imageView);
            break;

        case TextureType::Texture3D:
            GLTexImage3D(desc, imageView);
            break;

        case TextureType::TextureCube:
            GLTexImageCube(desc, imageView);
            break;

        #if LLGL_GLEXT_TEXTURE_MULTISAMPLE
        case TextureType::Texture1DArray:
            GLTexImage1DArray(desc, imageView);
            break;
        #endif // /LLGL_GLEXT_TEXTURE_MULTISAMPLE

        case TextureType::Texture2DArray:
            GLTexImage2DArray(desc, imageView);
            break;

        #if LLGL_GLEXT_TEXTURE_MULTISAMPLE
        case TextureType::TextureCubeArray:
            GLTexImageCubeArray(desc, imageView);
            break;

        case TextureType::Texture2DMS:
            GLTexImage2DMS(desc);
            break;

        case TextureType::Texture2DMSArray:
            GLTexImage2DMSArray(desc);
            break;
        #endif // /LLGL_GLEXT_TEXTURE_MULTISAMPLE

        default:
            return false;
    }

    return true;
}


} // /namespace LLGL



// ================================================================================
