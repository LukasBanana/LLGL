/*
 * Format.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_FORMAT_H
#define LLGL_FORMAT_H


#include <LLGL/Export.h>
#include <cstdint>
#include <cstddef>


namespace LLGL
{


/* ----- Enumerations ----- */

/**
\brief Texture and vertex attribute format enumeration used for GPU side operations.

\remarks The counterpart of this enumeration for CPU side operations are LLGL::ImageFormat and LLGL::DataType.

\remarks Packed color formats are stored from least significant bit (LSB) to most significant bit (MSB),
i.e. for the Format::RGB10A2UNorm the bit pattern is <code>A[31:30], B[29:20], G[19:10], R[9:0]</code> for instance.

\remarks The following table illustrates the bit-size for each component of all ordinary and packed color formats (depth-stencil and compressed formats excluded) with the following notation:
- <b>\c n</b> denotes unsigned normalized formats (\c UNorm)
- <b>\c s</b> denotes signed normalized formats (\c SNorm)
- <b>\c u</b> denotes unsigned integer formats (\c UInt)
- <b>\c i</b> denotes signed integer formats (\c SInt)
- <b>\c f</b> denotes floating-point formats (\c Float)

\remarks
| Format | Bits | Red | Green | Blue | Alpha | Shared |
|--------|-----:|----:|------:|-----:|------:|-------:|
| Format::A8UNorm         | <b>\c   8</b> |        |        |        | \c  n8 |        |
| Format::R8UNorm         | <b>\c   8</b> | \c  n8 |        |        |        |        |
| Format::R8SNorm         | <b>\c   8</b> | \c  s8 |        |        |        |        |
| Format::R8UInt          | <b>\c   8</b> | \c  u8 |        |        |        |        |
| Format::R8SInt          | <b>\c   8</b> | \c  i8 |        |        |        |        |
| Format::R16UNorm        | <b>\c  16</b> | \c n16 |        |        |        |        |
| Format::R16SNorm        | <b>\c  16</b> | \c s16 |        |        |        |        |
| Format::R16UInt         | <b>\c  16</b> | \c u16 |        |        |        |        |
| Format::R16SInt         | <b>\c  16</b> | \c i16 |        |        |        |        |
| Format::R16Float        | <b>\c  16</b> | \c f16 |        |        |        |        |
| Format::R32UInt         | <b>\c  32</b> | \c u32 |        |        |        |        |
| Format::R32SInt         | <b>\c  32</b> | \c i32 |        |        |        |        |
| Format::R32Float        | <b>\c  32</b> | \c f32 |        |        |        |        |
| Format::R64Float        | <b>\c  64</b> | \c f64 |        |        |        |        |
| Format::RG8UNorm        | <b>\c  16</b> | \c  n8 | \c  n8 |        |        |        |
| Format::RG8SNorm        | <b>\c  16</b> | \c  s8 | \c  s8 |        |        |        |
| Format::RG8UInt         | <b>\c  16</b> | \c  u8 | \c  u8 |        |        |        |
| Format::RG8SInt         | <b>\c  16</b> | \c  i8 | \c  i8 |        |        |        |
| Format::RG16UNorm       | <b>\c  32</b> | \c n16 | \c n16 |        |        |        |
| Format::RG16SNorm       | <b>\c  32</b> | \c s16 | \c s16 |        |        |        |
| Format::RG16UInt        | <b>\c  32</b> | \c u16 | \c u16 |        |        |        |
| Format::RG16SInt        | <b>\c  32</b> | \c i16 | \c i16 |        |        |        |
| Format::RG16Float       | <b>\c  32</b> | \c f16 | \c f16 |        |        |        |
| Format::RG32UInt        | <b>\c  64</b> | \c u32 | \c u32 |        |        |        |
| Format::RG32SInt        | <b>\c  64</b> | \c i32 | \c i32 |        |        |        |
| Format::RG32Float       | <b>\c  64</b> | \c f32 | \c f32 |        |        |        |
| Format::RG64Float       | <b>\c 128</b> | \c f64 | \c f64 |        |        |        |
| Format::RGB8UNorm       | <b>\c  24</b> | \c  n8 | \c  n8 | \c  n8 |        |        |
| Format::RGB8UNorm_sRGB  | <b>\c  24</b> | \c  n8 | \c  n8 | \c  n8 |        |        |
| Format::RGB8SNorm       | <b>\c  24</b> | \c  s8 | \c  s8 | \c  s8 |        |        |
| Format::RGB8UInt        | <b>\c  24</b> | \c  u8 | \c  u8 | \c  u8 |        |        |
| Format::RGB8SInt        | <b>\c  24</b> | \c  i8 | \c  i8 | \c  i8 |        |        |
| Format::RGB16UNorm      | <b>\c  48</b> | \c n16 | \c n16 | \c n16 |        |        |
| Format::RGB16SNorm      | <b>\c  48</b> | \c s16 | \c s16 | \c s16 |        |        |
| Format::RGB16UInt       | <b>\c  48</b> | \c u16 | \c u16 | \c u16 |        |        |
| Format::RGB16SInt       | <b>\c  48</b> | \c i16 | \c i16 | \c i16 |        |        |
| Format::RGB16Float      | <b>\c  48</b> | \c f16 | \c f16 | \c f16 |        |        |
| Format::RGB32UInt       | <b>\c  96</b> | \c u32 | \c u32 | \c u32 |        |        |
| Format::RGB32SInt       | <b>\c  96</b> | \c i32 | \c i32 | \c i32 |        |        |
| Format::RGB32Float      | <b>\c  96</b> | \c f32 | \c f32 | \c f32 |        |        |
| Format::RGB64Float      | <b>\c 192</b> | \c f64 | \c f64 | \c f64 |        |        |
| Format::RGBA8UNorm      | <b>\c  32</b> | \c  n8 | \c  n8 | \c  n8 | \c  n8 |        |
| Format::RGBA8UNorm_sRGB | <b>\c  32</b> | \c  n8 | \c  n8 | \c  n8 | \c  n8 |        |
| Format::RGBA8SNorm      | <b>\c  32</b> | \c  s8 | \c  s8 | \c  s8 | \c  s8 |        |
| Format::RGBA8UInt       | <b>\c  32</b> | \c  u8 | \c  u8 | \c  u8 | \c  u8 |        |
| Format::RGBA8SInt       | <b>\c  32</b> | \c  i8 | \c  i8 | \c  i8 | \c  i8 |        |
| Format::RGBA16UNorm     | <b>\c  64</b> | \c n16 | \c n16 | \c n16 | \c n16 |        |
| Format::RGBA16SNorm     | <b>\c  64</b> | \c s16 | \c s16 | \c s16 | \c s16 |        |
| Format::RGBA16UInt      | <b>\c  64</b> | \c u16 | \c u16 | \c u16 | \c u16 |        |
| Format::RGBA16SInt      | <b>\c  64</b> | \c i16 | \c i16 | \c i16 | \c i16 |        |
| Format::RGBA16Float     | <b>\c  64</b> | \c f16 | \c f16 | \c f16 | \c f16 |        |
| Format::RGBA32UInt      | <b>\c 128</b> | \c u32 | \c u32 | \c u32 | \c u32 |        |
| Format::RGBA32SInt      | <b>\c 128</b> | \c i32 | \c i32 | \c i32 | \c i32 |        |
| Format::RGBA32Float     | <b>\c 128</b> | \c f32 | \c f32 | \c f32 | \c f32 |        |
| Format::RGBA64Float     | <b>\c 256</b> | \c f64 | \c f64 | \c f64 | \c f64 |        |
| Format::BGRA8UNorm      | <b>\c  32</b> | \c  n8 | \c  n8 | \c  n8 | \c  n8 |        |
| Format::BGRA8UNorm_sRGB | <b>\c  32</b> | \c  n8 | \c  n8 | \c  n8 | \c  n8 |        |
| Format::BGRA8SNorm      | <b>\c  32</b> | \c  s8 | \c  s8 | \c  s8 | \c  s8 |        |
| Format::BGRA8UInt       | <b>\c  32</b> | \c  u8 | \c  u8 | \c  u8 | \c  u8 |        |
| Format::BGRA8SInt       | <b>\c  32</b> | \c  i8 | \c  i8 | \c  i8 | \c  i8 |        |
| Format::RGB10A2UNorm    | <b>\c  32</b> | \c n10 | \c n10 | \c n10 | \c  n2 |        |
| Format::RGB10A2UInt     | <b>\c  32</b> | \c u10 | \c u10 | \c u10 | \c  u2 |        |
| Format::RG11B10Float    | <b>\c  32</b> | \c f11 | \c f11 | \c f10 |        |        |
| Format::RGB9E5Float     | <b>\c  32</b> | \c   9 | \c   9 | \c   9 |        | \c   5 |

\see TextureDescriptor::format
\see VertexAttribute::format
\see RenderingCapabilities::textureFormats
\see OpenGL counterpart: https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glTexImage1D.xhtml#id-1.6.14.1
\see Vulkan counterpart \c VkFormat: https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/VkFormat.html
\see Direct3D counterpart \c DXGI_FORMAT: https://msdn.microsoft.com/en-us/library/windows/desktop/bb173059(v=vs.85).aspx
\see Metal counterpart for textures \c MTLPixelFormat: https://developer.apple.com/documentation/metal/mtlpixelformat
\see Metal counterpart for vertices \c MTLVertexFormat: https://developer.apple.com/documentation/metal/mtlvertexformat
*/
enum class Format
{
    Undefined,          //!< Undefined format.

    /* --- Alpha channel color formats --- */
    A8UNorm,            //!< Alpha channel format: alpha 8-bit normalized unsigned integer component.

    /* --- Red channel color formats --- */
    R8UNorm,            //!< Ordinary color format: red 8-bit normalized unsigned integer component.
    R8SNorm,            //!< Ordinary color format: red 8-bit normalized signed integer component.
    R8UInt,             //!< Ordinary color format: red 8-bit unsigned integer component.
    R8SInt,             //!< Ordinary color format: red 8-bit signed integer component.

    R16UNorm,           //!< Ordinary color format: red 16-bit normalized unsigned interger component.
    R16SNorm,           //!< Ordinary color format: red 16-bit normalized signed interger component.
    R16UInt,            //!< Ordinary color format: red 16-bit unsigned interger component.
    R16SInt,            //!< Ordinary color format: red 16-bit signed interger component.
    R16Float,           //!< Ordinary color format: red 16-bit floating point component.

    R32UInt,            //!< Ordinary color format: red 32-bit unsigned interger component.
    R32SInt,            //!< Ordinary color format: red 32-bit signed interger component.
    R32Float,           //!< Ordinary color format: red 32-bit floating point component.

    R64Float,           //!< Ordinary color format: red 64-bit floating point component. \note Only supported with: Vulkan.

    /* --- RG color formats --- */
    RG8UNorm,           //!< Ordinary color format: red, green 8-bit normalized unsigned integer components.
    RG8SNorm,           //!< Ordinary color format: red, green 8-bit normalized signed integer components.
    RG8UInt,            //!< Ordinary color format: red, green 8-bit unsigned integer components.
    RG8SInt,            //!< Ordinary color format: red, green 8-bit signed integer components.

    RG16UNorm,          //!< Ordinary color format: red, green 16-bit normalized unsigned interger components.
    RG16SNorm,          //!< Ordinary color format: red, green 16-bit normalized signed interger components.
    RG16UInt,           //!< Ordinary color format: red, green 16-bit unsigned interger components.
    RG16SInt,           //!< Ordinary color format: red, green 16-bit signed interger components.
    RG16Float,          //!< Ordinary color format: red, green 16-bit floating point components.

    RG32UInt,           //!< Ordinary color format: red, green 32-bit unsigned interger components.
    RG32SInt,           //!< Ordinary color format: red, green 32-bit signed interger components.
    RG32Float,          //!< Ordinary color format: red, green 32-bit floating point components.

    RG64Float,          //!< Ordinary color format: red, green 64-bit floating point components. \note Only supported with: Vulkan.

    /* --- RGB color formats --- */
    RGB8UNorm,          //!< Ordinary color format: red, green, blue 8-bit normalized unsigned integer components. \note Only supported with: OpenGL, Vulkan.
    RGB8UNorm_sRGB,     //!< Ordinary color format: red, green, blue 8-bit normalized unsigned integer components in non-linear sRGB color space. \note Only supported with: OpenGL, Vulkan.
    RGB8SNorm,          //!< Ordinary color format: red, green, blue 8-bit normalized signed integer components. \note Only supported with: OpenGL, Vulkan.
    RGB8UInt,           //!< Ordinary color format: red, green, blue 8-bit unsigned integer components. \note Only supported with: OpenGL, Vulkan.
    RGB8SInt,           //!< Ordinary color format: red, green, blue 8-bit signed integer components. \note Only supported with: OpenGL, Vulkan.

    RGB16UNorm,         //!< Ordinary color format: red, green, blue 16-bit normalized unsigned interger components. \note Only supported with: OpenGL, Vulkan.
    RGB16SNorm,         //!< Ordinary color format: red, green, blue 16-bit normalized signed interger components. \note Only supported with: OpenGL, Vulkan.
    RGB16UInt,          //!< Ordinary color format: red, green, blue 16-bit unsigned interger components. \note Only supported with: OpenGL, Vulkan.
    RGB16SInt,          //!< Ordinary color format: red, green, blue 16-bit signed interger components. \note Only supported with: OpenGL, Vulkan.
    RGB16Float,         //!< Ordinary color format: red, green, blue 16-bit floating point components. \note Only supported with: OpenGL, Vulkan.

    RGB32UInt,          //!< Ordinary color format: red, green, blue 32-bit unsigned interger components. \note As texture format only supported with: OpenGL, Vulkan, Direct3D 11, Direct3D 12.
    RGB32SInt,          //!< Ordinary color format: red, green, blue 32-bit signed interger components. \note As texture format only supported with: OpenGL, Vulkan, Direct3D 11, Direct3D 12.
    RGB32Float,         //!< Ordinary color format: red, green, blue 32-bit floating point components. \note As texture format only supported with: OpenGL, Vulkan, Direct3D 11, Direct3D 12.

    RGB64Float,         //!< Ordinary color format: red, green, blue 64-bit floating point components. \note Only supported with: Vulkan.

    /* --- RGBA color formats --- */
    RGBA8UNorm,         //!< Ordinary color format: red, green, blue, alpha 8-bit normalized unsigned integer components.
    RGBA8UNorm_sRGB,    //!< Ordinary color format: red, green, blue, alpha 8-bit normalized unsigned integer components in non-linear sRGB color space.
    RGBA8SNorm,         //!< Ordinary color format: red, green, blue, alpha 8-bit normalized signed integer components.
    RGBA8UInt,          //!< Ordinary color format: red, green, blue, alpha 8-bit unsigned integer components.
    RGBA8SInt,          //!< Ordinary color format: red, green, blue, alpha 8-bit signed integer components.

    RGBA16UNorm,        //!< Ordinary color format: red, green, blue, alpha 16-bit normalized unsigned interger components.
    RGBA16SNorm,        //!< Ordinary color format: red, green, blue, alpha 16-bit normalized signed interger components.
    RGBA16UInt,         //!< Ordinary color format: red, green, blue, alpha 16-bit unsigned interger components.
    RGBA16SInt,         //!< Ordinary color format: red, green, blue, alpha 16-bit signed interger components.
    RGBA16Float,        //!< Ordinary color format: red, green, blue, alpha 16-bit floating point components.

    RGBA32UInt,         //!< Ordinary color format: red, green, blue, alpha 32-bit unsigned interger components.
    RGBA32SInt,         //!< Ordinary color format: red, green, blue, alpha 32-bit signed interger components.
    RGBA32Float,        //!< Ordinary color format: red, green, blue, alpha 32-bit floating point components.

    RGBA64Float,        //!< Ordinary color format: red, green, blue, alpha 64-bit floating point components. \note Only supported with: Vulkan.

    /* --- BGRA color formats --- */
    BGRA8UNorm,         //!< Ordinary color format: blue, green, red, alpha 8-bit normalized unsigned integer components.
    BGRA8UNorm_sRGB,    //!< Ordinary color format: blue, green, red, alpha 8-bit normalized unsigned integer components in non-linear sRGB color space.
    BGRA8SNorm,         //!< Ordinary color format: blue, green, red, alpha 8-bit normalized signed integer components. \note Only supported with: Vulkan.
    BGRA8UInt,          //!< Ordinary color format: blue, green, red, alpha 8-bit unsigned integer components. \note Only supported with: Vulkan.
    BGRA8SInt,          //!< Ordinary color format: blue, green, red, alpha 8-bit signed integer components. \note Only supported with: Vulkan.

    /* --- Packed formats --- */
    RGB10A2UNorm,       //!< Packed color format: red, green, blue 10-bit and alpha 2-bit normalized unsigned integer components.
    RGB10A2UInt,        //!< Packed color format: red, green, blue 10-bit and alpha 2-bit unsigned integer components.
    RG11B10Float,       //!< Packed color format: red, green 11-bit and blue 10-bit unsigned floating point, i.e. 6-bit mantissa for red and green, 5-bit mantissa for blue, and 5-bit exponent for all components.
    RGB9E5Float,        //!< Packed color format: red, green, blue 9-bit unsigned floating-point with shared 5-bit exponent, i.e. 9-bit mantissa for each component and one 5-bit exponent for all components.

    /* --- Depth-stencil formats --- */
    D16UNorm,           //!< Depth-stencil format: depth 16-bit normalized unsigned integer component.
    D24UNormS8UInt,     //!< Depth-stencil format: depth 24-bit normalized unsigned integer component, and 8-bit unsigned integer stencil component.
    D32Float,           //!< Depth-stencil format: depth 32-bit floating point component.
    D32FloatS8X24UInt,  //!< Depth-stencil format: depth 32-bit floating point component, and 8-bit unsigned integer stencil components (where the remaining 24 bits are unused).
  //S8UInt,             //!< Stencil only format: 8-bit unsigned integer stencil component. \note Only supported with: OpenGL, Vulkan, Metal.

    /* --- Block compression (BC) formats --- */
    BC1UNorm,           //!< Compressed color format: S3TC BC1 compressed RGBA with normalized unsigned integer components in 64-bit per 4x4 block.
    BC1UNorm_sRGB,      //!< Compressed color format: S3TC BC1 compressed RGBA with normalized unsigned integer components in 64-bit per 4x4 block in non-linear sRGB color space.
    BC2UNorm,           //!< Compressed color format: S3TC BC2 compressed RGBA with normalized unsigned integer components in 128-bit per 4x4 block.
    BC2UNorm_sRGB,      //!< Compressed color format: S3TC BC2 compressed RGBA with normalized unsigned integer components in 128-bit per 4x4 block in non-linear sRGB color space.
    BC3UNorm,           //!< Compressed color format: S3TC BC3 compressed RGBA with normalized unsigned integer components in 128-bit per 4x4 block.
    BC3UNorm_sRGB,      //!< Compressed color format: S3TC BC3 compressed RGBA with normalized unsigned integer components in 128-bit per 4x4 block in non-linear sRGB color space.
    BC4UNorm,           //!< Compressed color format: S3TC BC4 compressed red channel with normalized unsigned integer component in 64-bit per 4x4 block.
    BC4SNorm,           //!< Compressed color format: S3TC BC4 compressed red channel with normalized signed integer component 64-bit per 4x4 block.
    BC5UNorm,           //!< Compressed color format: S3TC BC5 compressed red and green channels with normalized unsigned integer components in 64-bit per 4x4 block.
    BC5SNorm,           //!< Compressed color format: S3TC BC5 compressed red and green channels with normalized signed integer components in 128-bit per 4x4 block.
};

/**
\brief Image format enumeration that applies to each pixel of an image.
\see SrcImageDescriptor::format
\see DstImageDescriptor::format
\see GetMemoryFootprint
\todo Maybe replace \c DataType and \c ImageFormat by LLGL::Format.
*/
enum class ImageFormat
{
    /* Color formats */
    Alpha,          //!< Single color component: Alpha.
    R,              //!< Single color component: Red.
    RG,             //!< Two color components: Red, Green.
    RGB,            //!< Three color components: Red, Green, Blue.
    BGR,            //!< Three color components: Blue, Green, Red.
    RGBA,           //!< Four color components: Red, Green, Blue, Alpha.
    BGRA,           //!< Four color components: Blue, Green, Red, Alpha.
    ARGB,           //!< Four color components: Alpha, Red, Green, Blue. Legacy format, mainly used in Direct3D 9.
    ABGR,           //!< Four color components: Alpha, Blue, Green, Red. Legacy format, mainly used in Direct3D 9.

    /* Depth-stencil formats */
    Depth,          //!< Depth component.
    DepthStencil,   //!< Depth and stencil components.
    Stencil,        //!< Stencil component.

    /* Compressed formats */
    BC1,            //!< Block compression BC1.
    BC2,            //!< Block compression BC2.
    BC3,            //!< Block compression BC3.
    BC4,            //!< Block compression BC4.
    BC5,            //!< Block compression BC5.
};

/**
\brief Data types enumeration used for CPU side operations.
\remarks The counterpart of this enumeration for GPU side operations is LLGL::Format.
\see SrcImageDescriptor::dataType
\see DstImageDescriptor::dataType
\todo Maybe replace \c DataType and \c ImageFormat by LLGL::Format.
*/
enum class DataType
{
    Undefined,  //!< Undefined data type.

    Int8,       //!< 8-bit signed integer (\c char).
    UInt8,      //!< 8-bit unsigned integer (<c>unsigned char</c>).

    Int16,      //!< 16-bit signed integer (\c short).
    UInt16,     //!< 16-bit unsigned integer (<c>unsigned short</c>).

    Int32,      //!< 32-bit signed integer (\c int).
    UInt32,     //!< 32-bit unsigned integer (<c>unsigned int</c>).

    Float16,    //!< 16-bit floating-point (\c half).
    Float32,    //!< 32-bit floating-point (\c float).
    Float64,    //!< 64-bit real type (\c double).
};

/**
\brief Format attribute flags enumeration.
\see FormatAttributes::flags
*/
struct FormatFlags
{
    enum
    {
        //! Specifies whether the format has a depth component (e.g. Format::D16UNorm or Format::D24UNormS8UInt).
        HasDepth                = (1 << 0),

        //! Specifies whether the format has a stencil component (e.g. Format::D24UNormS8UInt).
        HasStencil              = (1 << 1),

        //! Specifies whether the format is in non-linear sRGB color space (e.g. Format::RGBA8UNorm_sRGB or Format::BC1UNorm_sRGB).
        IsColorSpace_sRGB       = (1 << 2),

        //! Specifies whether the format is compressed (e.g. Format::BC1UNorm or Format::BC5SNorm).
        IsCompressed            = (1 << 3),

        //! Specifies whether the format has normalized integer components (e.g. Format::R8UNorm or Format::R8SNorm).
        IsNormalized            = (1 << 4),

        //! Specifies whether the format is an integer format (e.g. Format::R8UNorm or Format::R8SInt).
        IsInteger               = (1 << 5),

        //! Specifies whether the format is an unsigned format (e.g. Format::R8UInt or Format::RG11B10Float).
        IsUnsigned              = (1 << 6),

        //! Specifies whether the format is packed (e.g. Format::RGB10A2UNorm or Format::RGB9E5Float).
        IsPacked                = (1 << 7),

        /**
        \brief Specifies whether the format can be used for render targets or rather depth-stencil or color attachments.
        \see BindFlags::ColorAttachment
        \see BindFlags::DepthStencilAttachment
        */
        SupportsRenderTarget    = (1 << 8),

        /**
        \brief Specifies whether the format can have more than one MIP-map.
        \see TextureDescriptor::mipLevels
        */
        SupportsMips            = (1 << 9),

        /**
        \brief Specifies whether the format supports to automatically generate MIP-maps.
        \remarks This implies that the flags \c SupportsMips and \c SupportsRenderTarget are also enabled.
        \see MiscFlgas::GenerateMips
        \see CommandBuffer::GenerateMips
        */
        SupportsGenerateMips    = (1 << 10),

        /**
        \brief Specifies whether the format can be used for 1D and 1D-array textures.
        \remarks For example, block compressions (e.g. Format::BC1UNorm) cannot be used for 1D textures, because they are made out of 4x4 blocks.
        \see TextureType::Texture1D
        \see TextureType::Texture1DArray
        */
        SupportsTexture1D       = (1 << 11),

        /**
        \brief Specifies whether the format can be used for 2D and 2D-array textures (both regular and multisampled).
        \see TextureType::Texture2D
        \see TextureType::Texture2DArray
        \see TextureType::Texture2DMS
        \see TextureType::Texture2DMSArray
        */
        SupportsTexture2D       = (1 << 12),

        /**
        \brief Specifies whether the format can be used for 3D textures.
        \see TextureType::Texture3D
        */
        SupportsTexture3D       = (1 << 13),

        /**
        \brief Specifies whether the format can be used for cube and cube-array textures.
        \see TextureType::TextureCube
        \see TextureType::TextureCubeArray
        */
        SupportsTextureCube     = (1 << 14),

        /**
        \brief Specifies whether the format can be used for vertex attributes.
        \see VertexAttribute::format
        */
        SupportsVertex          = (1 << 15),

        //! Combines the format flags \c IsInteger and \c IsUnsigned.
        IsUnsignedInteger       = (IsUnsigned | IsInteger),

        //! Combines the format flags \c HasDepth and \c HasStencil.
        HasDepthStencil         = (HasDepth | HasStencil),
    };
};


/* ----- Structures ----- */

/**
\brief Vertex and texture format attributes structure.
\remarks Describes all attributes of a hardware format, e.g. bit size, color components, compression etc.
\note This descriptor structure has no default initializers to fulfill the requirements of a plain-old-data (POD) structure.
\see GetFormatAttribs
*/
struct FormatAttributes
{
    //! Size (in bits) of the a pixel block or vertex attribute. This is 0 for invalid formats.
    std::uint16_t   bitSize;

    //! Width of a pixel block. This is 1 for all non-compressed formats. This is 0 for invalid formats.
    std::uint8_t    blockWidth;

    //! Height of a pixel block. This is 1 for all non-compressed formats. This is 0 for invalid formats.
    std::uint8_t    blockHeight;

    //! Number of components. This is either 1, 2, 3, or 4. This is 0 for invalid formats.
    std::uint8_t    components;

    //! Image pixel format. This is ignored for depth-stencil and compressed formats.
    ImageFormat     format;

    /**
    \brief Color component data type. This is ignored for depth-stencil and compressed formats.
    \remarks For depth-stencil formats, this data type always refers to the depth component, because stencil components are always in 8-bit unsigned integer format.
    */
    DataType        dataType;

    /**
    \brief Specifies the boolean attributes of this format descriptor. This can be bitwise OR combination of the FormatFlags entries.
    \see FormatFlags
    */
    long            flags;
};


/* ----- Functions ----- */

/**
\defgroup group_format_util Hardware format utility functions.
\addtogroup group_format_util
@{
*/

/**
\brief Returns the attributes for the specified hardware format or a zero-initialized structure in case of an invalid argument.
\see FormatAttributes
\see Format
*/
LLGL_EXPORT const FormatAttributes& GetFormatAttribs(const Format format);

/**
\brief Returns the size (in number of components) of the specified image format.
\param[in] imageFormat Specifies the image format.
\return Number of components of the specified image format, or 0 if \c imageFormat specifies a compressed color format.
\note Compressed formats are not supported.
\see IsCompressedFormat(const ImageFormat)
\see ImageFormat
*/
LLGL_EXPORT std::uint32_t ImageFormatSize(const ImageFormat imageFormat);

/**
\brief Returns the memory footprint (in bytes) of a texture or buffer with the specified hardware format and number of elements.
\param[in] format Specifies the hardware format.
\param[in] numTexels Specifies the number of elements that make up the memory footprint.
For the DXT compressed formats, this must be a multiple of 16, since these formats compress images in 4x4 texel blocks.
\return The memory footprint size (in bytes), or zero if the input is invalid.
\see GetMemoryFootprint(const TextureType, const Format, const Extent3D&, const TextureSubresource&)
\see Texture::GetMemoryFootprint
*/
LLGL_EXPORT std::size_t GetMemoryFootprint(const Format format, std::size_t numTexels);

/**
\brief Returns the memory footprint (in bytes) of an image with the specified format, data type, and number of elements.
\param[in] imageFormat Specifies the image format.
\param[in] dataType Specifies the data type of each pixel component.
\param[in] numTexels Specifies the number of elements that make up the memory footprint.
*/
LLGL_EXPORT std::size_t GetMemoryFootprint(const ImageFormat imageFormat, const DataType dataType, std::size_t numTexels);

/**
\brief Returns true if the specified hardware format is a compressed format,
e.g. Format::BC1UNorm, Format::BC2UNorm_sRGB, Format::BC4SNorm, etc.
\see Format
*/
LLGL_EXPORT bool IsCompressedFormat(const Format format);

/**
\brief Returns true if the specified color format is a compressed format,
i.e. either ImageFormat::CompressedRGB, or ImageFormat::CompressedRGBA.
\see ImageFormat
*/
LLGL_EXPORT bool IsCompressedFormat(const ImageFormat imageFormat);

/**
\brief Returns true if the specified hardware format is a depth or depth-stencil format,
i.e. Format::D16UNorm, Format::D24UNormS8UInt, Format::D32Float, or Format::D32FloatS8X24UInt.
\see Format
\see IsDepthAndStencilFormat
*/
LLGL_EXPORT bool IsDepthOrStencilFormat(const Format format);

/**
\brief Returns true if the specified hardware format is a depth and stencil format,
i.e. Format::D24UNormS8UInt or Format::D32FloatS8X24UInt.
\see Format
\see IsDepthOrStencilFormat
*/
LLGL_EXPORT bool IsDepthAndStencilFormat(const Format format);

/**
\brief Returns true if the specified color format is a depth-stencil format,
i.e. either ImageFormat::Depth or ImageFormat::DepthStencil.
\see ImageFormat
*/
LLGL_EXPORT bool IsDepthOrStencilFormat(const ImageFormat imageFormat);

/**
\brief Returns true if the specified hardware format is a depth format,
i.e. Format::D16UNorm, Format::D24UNormS8UInt, Format::D32Float, or Format::D32FloatS8X24UInt.
\see Format
*/
LLGL_EXPORT bool IsDepthFormat(const Format format);

/**
\brief Returns true if the specified hardware format is a stencil format,
i.e. Format::D24UNormS8UInt or Format::D32FloatS8X24UInt.
\see Format
*/
LLGL_EXPORT bool IsStencilFormat(const Format format);

/**
\brief Returns true if the specified hardware format is a color format.
\remarks This is equivalent to <code>(format != Format::Undefined && !IsDepthOrStencilFormat(format))</code>.
\see Format
*/
LLGL_EXPORT bool IsColorFormat(const Format format);

/**
\brief Returns true if the specified hardware format is a normalized format (like Format::RGBA8UNorm, Format::R8SNorm etc.).
\remarks This does not include depth-stencil formats or compressed formats.
\see IsDepthOrStencilFormat
\see IsCompressedFormat
\see Format
*/
LLGL_EXPORT bool IsNormalizedFormat(const Format format);

/**
\brief Returns true if the specified hardware format is an integral format (like Format::RGBA8UInt, Format::RGBA8UNorm, Format::R8SInt etc.).
\remarks This also includes all normalized formats.
\see IsNormalizedFormat
\see Format
*/
LLGL_EXPORT bool IsIntegralFormat(const Format format);

/**
\brief Returns true if the specified hardware format is a floating-point format (like Format::RGBA32Float, Format::R32Float etc.).
\remarks This does not include depth-stencil formats or compressed formats.
\see IsDepthOrStencilFormat
\see IsCompressedFormat
\see Format
*/
LLGL_EXPORT bool IsFloatFormat(const Format format);

/** @} */

/**
\defgroup group_datatype_util Data type utility functions.
\addtogroup group_datatype_util
@{
*/

//! Returns the size (in bytes) of the specified data type.
LLGL_EXPORT std::uint32_t DataTypeSize(const DataType dataType);

/**
\brief Determines if the argument refers to a signed integer data type.
\return True if the specified data type equals one of the following enumeration entries: DataType::Int8, DataType::Int16, DataType::Int32.
*/
LLGL_EXPORT bool IsIntDataType(const DataType dataType);

/**
\brief Determines if the argument refers to an unsigned integer data type.
\return True if the specified data type equals one of the following enumeration entries: DataType::UInt8, DataType::UInt16, DataType::UInt32.
*/
LLGL_EXPORT bool IsUIntDataType(const DataType dataType);

/**
\brief Determines if the argument refers to a floating-pointer data type.
\return True if the specified data type equals one of the following enumeration entries: DataType::Float16, DataType::Float32, DataType::Float64.
*/
LLGL_EXPORT bool IsFloatDataType(const DataType dataType);

/** @} */


} // /namespace LLGL


#endif



// ================================================================================
