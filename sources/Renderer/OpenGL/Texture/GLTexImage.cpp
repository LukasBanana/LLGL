/*
 * GLTexImage.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLTexImage.h"
#include "../GLTypes.h"
#include "../GLProfile.h"
#include "../Ext/GLExtensions.h"
#include "../Ext/GLExtensionRegistry.h"
#include <array>
#include <algorithm>


namespace LLGL
{


// Generates an image buffer with floating-points for RGBA components.
static std::vector<ColorRGBAf> GenImageDataRGBAf(std::uint32_t numPixels, const ColorRGBAf& color)
{
    return std::vector<ColorRGBAf>(static_cast<std::size_t>(numPixels), color);
}

// Generates an image buffer with floating-points for the Red component.
static std::vector<float> GenImageDataRf(std::uint32_t numPixels, float value)
{
    return std::vector<float>(static_cast<std::size_t>(numPixels), value);
}

struct alignas(4) GLDepthStencilPair
{
    float           depth;
    std::uint8_t    stencil;
};

// Generates an image buffer with floating-points for the Red component and an unsigned byte for the Green component.
static std::vector<GLDepthStencilPair> GenImageDataD32fS8ui(std::uint32_t numPixels, float depth, std::uint8_t stencil)
{
    return std::vector<GLDepthStencilPair>(static_cast<std::size_t>(numPixels), GLDepthStencilPair{ depth, stencil });
}

// Returns true if the specified hardware format requires an integer type, e.g. GL_RGBA_INTEGER
static bool IsIntegerTypedFormat(const Format format)
{
    return ((GetFormatAttribs(format).flags & (FormatFlags::IsInteger | FormatFlags::IsNormalized)) == FormatFlags::IsInteger);
}

// Returns true if the 'clearValue' member is enabled if no initial image data is specified, i.e. MiscFlags::NoInitialData is NOT specified
static bool IsClearValueEnabled(const TextureDescriptor& desc)
{
    return ((desc.miscFlags & MiscFlags::NoInitialData) == 0);
}

// Returns true if a GL texture with the specified descriptor can be default initialized with an RGBA float format, i.e. GL_RGBA and GL_FLOAT.
static bool CanInitializeTexWithRGBAf(const TextureDescriptor& desc)
{
    return (IsClearValueEnabled(desc) && !IsCompressedFormat(desc.format) && !IsIntegerTypedFormat(desc.format));
}

[[noreturn]]
static void ErrIllegalUseOfDepthFormat()
{
    throw std::runtime_error("illegal use of depth-stencil format for texture");
}

// Converts the internal format if necessary
static Format FindSuitableDepthFormat(const TextureDescriptor& desc)
{
    if (IsDepthStencilFormat(desc.format))
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
            /* Use <internalFormat> for the compressed version, and <format> for the uncompressed version */
            if (IsCompressedFormat(textureFormat))
                glCompressedTexSubImage1D(target, 0, 0, sx, internalFormat, static_cast<GLsizei>(dataSize), data);
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
            /* Use <internalFormat> for the compressed version, and <format> for the uncompressed version */
            if (IsCompressedFormat(textureFormat))
                glCompressedTexSubImage2D(target, 0, 0, 0, sx, sy, internalFormat, static_cast<GLsizei>(dataSize), data);
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
            #ifdef LLGL_OPENGL
            if (target == GL_TEXTURE_1D_ARRAY || target == GL_PROXY_TEXTURE_1D_ARRAY)
            {
                for (std::uint32_t i = 1; i < mipLevels; ++i)
                {
                    sx = std::max(1u, sx / 2u);
                    glTexImage2D(target, static_cast<GLint>(i), internalFormat, sx, sy, 0, format, type, nullptr);
                }
            }
            else
            #endif
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
            /* Use <internalFormat> for the compressed version, and <format> for the uncompressed version */
            if (IsCompressedFormat(textureFormat))
                glCompressedTexSubImage3D(target, 0, 0, 0, 0, sx, sy, sz, internalFormat, static_cast<GLsizei>(dataSize), data);
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
    auto internalFormat         = GLTypes::Map(textureFormat);
    auto sampleCount            = static_cast<GLsizei>(samples);
    auto sx                     = static_cast<GLsizei>(width);
    auto sy                     = static_cast<GLsizei>(height);
    auto sz                     = static_cast<GLsizei>(depth);
    auto fixedSampleLocations   = static_cast<GLboolean>(fixedSamples ? GL_TRUE : GL_FALSE);

    #ifdef GL_ARB_texture_storage_multisample
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

#endif

#ifdef LLGL_OPENGL

static void GLTexImage1D(const TextureDescriptor& desc, const SrcImageDescriptor* imageDesc)
{
    if (imageDesc)
    {
        /* Setup texture image from descriptor */
        GLTexImage1D(
            NumMipLevels(desc),
            desc.format,
            desc.extent.width,
            GLTypes::Map(imageDesc->format, IsIntegerTypedFormat(desc.format)),
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
            (IsIntegerTypedFormat(desc.format) ? GL_RGBA_INTEGER : GL_RGBA),
            GL_UNSIGNED_BYTE,
            nullptr
        );
    }
}

#endif

static void GLTexImage2D(const TextureDescriptor& desc, const SrcImageDescriptor* imageDesc)
{
    if (imageDesc)
    {
        /* Setup texture image from descriptor */
        GLTexImage2D(
            NumMipLevels(desc),
            desc.format,
            desc.extent.width,
            desc.extent.height,
            GLTypes::Map(imageDesc->format, IsIntegerTypedFormat(desc.format)),
            GLTypes::Map(imageDesc->dataType),
            imageDesc->data,
            imageDesc->dataSize
        );
    }
    else if (IsStencilFormat(desc.format))
    {
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
            (IsIntegerTypedFormat(desc.format) ? GL_RGBA_INTEGER : GL_RGBA),
            GL_UNSIGNED_BYTE,
            nullptr
        );
    }
}

static void GLTexImage3D(const TextureDescriptor& desc, const SrcImageDescriptor* imageDesc)
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
            GLTypes::Map(imageDesc->format, IsIntegerTypedFormat(desc.format)),
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
            (IsIntegerTypedFormat(desc.format) ? GL_RGBA_INTEGER : GL_RGBA),
            GL_UNSIGNED_BYTE,
            nullptr
        );
    }
}

static void GLTexImageCube(const TextureDescriptor& desc, const SrcImageDescriptor* imageDesc)
{
    const auto numMipLevels = NumMipLevels(desc);

    if (imageDesc)
    {
        /* Setup texture image cube-faces from descriptor */
        auto imageFace          = reinterpret_cast<const char*>(imageDesc->data);
        auto imageFaceStride    = (desc.extent.width * desc.extent.height * ImageFormatSize(imageDesc->format) * DataTypeSize(imageDesc->dataType));

        if (IsCompressedFormat(desc.format))
            imageFaceStride = static_cast<std::uint32_t>(imageDesc->dataSize);

        auto dataFormatGL       = GLTypes::Map(imageDesc->format, IsIntegerTypedFormat(desc.format));
        auto dataTypeGL         = GLTypes::Map(imageDesc->dataType);

        for (std::uint32_t arrayLayer = 0; arrayLayer < desc.arrayLayers; ++arrayLayer)
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
                imageDesc->dataSize
            );
            imageFace += imageFaceStride;
        }
    }
    else if (IsStencilFormat(desc.format))
    {
        auto internalFormat = FindSuitableDepthFormat(desc);

        std::vector<GLDepthStencilPair> image;
        const void* initialData = nullptr;

        if (IsClearValueEnabled(desc))
        {
            /* Initialize depth texture image with default depth */
            image       = GenImageDataD32fS8ui(desc.extent.width * desc.extent.height, desc.clearValue.depth, desc.clearValue.stencil);
            initialData = image.data();
        }

        /* Allocate depth texture image without initial data */
        for (std::uint32_t arrayLayer = 0; arrayLayer < desc.arrayLayers; ++arrayLayer)
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
    }
    else if (IsDepthFormat(desc.format))
    {
        auto internalFormat = FindSuitableDepthFormat(desc);

        std::vector<float> image;
        const void* initialData = nullptr;

        if (IsClearValueEnabled(desc))
        {
            /* Initialize depth texture image with default depth */
            image       = GenImageDataRf(desc.extent.width * desc.extent.height, desc.clearValue.depth);
            initialData = image.data();
        }

        /* Allocate depth texture image without initial data */
        for (std::uint32_t arrayLayer = 0; arrayLayer < desc.arrayLayers; ++arrayLayer)
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
        for (std::uint32_t arrayLayer = 0; arrayLayer < desc.arrayLayers; ++arrayLayer)
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
        for (std::uint32_t arrayLayer = 0; arrayLayer < desc.arrayLayers; ++arrayLayer)
        {
            GLTexImageCube(
                numMipLevels,
                desc.format,
                desc.extent.width,
                desc.extent.height,
                arrayLayer,
                (IsIntegerTypedFormat(desc.format) ? GL_RGBA_INTEGER : GL_RGBA),
                GL_UNSIGNED_BYTE,
                nullptr
            );
        }
    }
}

#ifdef LLGL_OPENGL

static void GLTexImage1DArray(const TextureDescriptor& desc, const SrcImageDescriptor* imageDesc)
{
    if (imageDesc)
    {
        /* Setup texture image from descriptor */
        GLTexImage1DArray(
            NumMipLevels(desc),
            desc.format,
            desc.extent.width,
            desc.arrayLayers,
            GLTypes::Map(imageDesc->format, IsIntegerTypedFormat(desc.format)),
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
            (IsIntegerTypedFormat(desc.format) ? GL_RGBA_INTEGER : GL_RGBA),
            GL_UNSIGNED_BYTE,
            nullptr
        );
    }
}

#endif

static void GLTexImage2DArray(const TextureDescriptor& desc, const SrcImageDescriptor* imageDesc)
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
            GLTypes::Map(imageDesc->format, IsIntegerTypedFormat(desc.format)),
            GLTypes::Map(imageDesc->dataType),
            imageDesc->data,
            imageDesc->dataSize
        );
    }
    else if (IsStencilFormat(desc.format))
    {
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
            (IsIntegerTypedFormat(desc.format) ? GL_RGBA_INTEGER : GL_RGBA),
            GL_UNSIGNED_BYTE,
            nullptr
        );
    }
}

#ifdef LLGL_OPENGL

static void GLTexImageCubeArray(const TextureDescriptor& desc, const SrcImageDescriptor* imageDesc)
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
            GLTypes::Map(imageDesc->format, IsIntegerTypedFormat(desc.format)),
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
            (IsIntegerTypedFormat(desc.format) ? GL_RGBA_INTEGER : GL_RGBA),
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

#endif

void GLTexImage(const TextureDescriptor& desc, const SrcImageDescriptor* imageDesc)
{
    switch (desc.type)
    {
        #ifdef LLGL_OPENGL
        case TextureType::Texture1D:
            GLTexImage1D(desc, imageDesc);
            break;
        #endif

        case TextureType::Texture2D:
            GLTexImage2D(desc, imageDesc);
            break;

        case TextureType::Texture3D:
            GLTexImage3D(desc, imageDesc);
            break;

        case TextureType::TextureCube:
            GLTexImageCube(desc, imageDesc);
            break;

        #ifdef LLGL_OPENGL
        case TextureType::Texture1DArray:
            GLTexImage1DArray(desc, imageDesc);
            break;
        #endif

        case TextureType::Texture2DArray:
            GLTexImage2DArray(desc, imageDesc);
            break;

        #ifdef LLGL_OPENGL
        case TextureType::TextureCubeArray:
            GLTexImageCubeArray(desc, imageDesc);
            break;

        case TextureType::Texture2DMS:
            GLTexImage2DMS(desc);
            break;

        case TextureType::Texture2DMSArray:
            GLTexImage2DMSArray(desc);
            break;
        #endif

        default:
            break;
    }
}


} // /namespace LLGL



// ================================================================================
