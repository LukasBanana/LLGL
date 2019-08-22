/*
 * Format.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_FORMAT_H
#define LLGL_FORMAT_H


#include "Export.h"
#include <cstdint>


namespace LLGL
{


/* ----- Enumerations ----- */

/**
\brief Texture and vertex attribute format enumeration used for GPU side operations.
\remarks The counterpart of this enumeration for CPU side operations are LLGL::ImageFormat and LLGL::DataType.
\see TextureDescriptor::format
\see VertexAttribute::format
\see RenderingCapabilities::textureFormats
\see OpenGL counterpart: https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glTexImage1D.xhtml#id-1.6.14.1
\see Vulkan counterpart <code>VkFormat</code>: https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/VkFormat.html
\see Direct3D counterpart <code>DXGI_FORMAT</code>: https://msdn.microsoft.com/en-us/library/windows/desktop/bb173059(v=vs.85).aspx
\see Metal counterpart <code>MTLPixelFormat</code>: https://developer.apple.com/documentation/metal/mtlpixelformat
*/
enum class Format
{
    Undefined,          //!< Undefined format.

    /* --- Alpha channel color formats --- */
    A8UNorm,            //!< Alpha channel format: alpha 8-bit normalized unsigned integer component.

    /* --- Red channel color formats --- */
    R8UNorm,            //!< Color format: red 8-bit normalized unsigned integer component.
    R8SNorm,            //!< Color format: red 8-bit normalized signed integer component.
    R8UInt,             //!< Color format: red 8-bit unsigned integer component.
    R8SInt,             //!< Color format: red 8-bit signed integer component.

    R16UNorm,           //!< Color format: red 16-bit normalized unsigned interger component.
    R16SNorm,           //!< Color format: red 16-bit normalized signed interger component.
    R16UInt,            //!< Color format: red 16-bit unsigned interger component.
    R16SInt,            //!< Color format: red 16-bit signed interger component.
    R16Float,           //!< Color format: red 16-bit floating point component.

    R32UInt,            //!< Color format: red 32-bit unsigned interger component.
    R32SInt,            //!< Color format: red 32-bit signed interger component.
    R32Float,           //!< Color format: red 32-bit floating point component.

    R64Float,           //!< Color format: red 64-bit floating point component. \note Only supported with: Vulkan.

    /* --- RG color formats --- */
    RG8UNorm,           //!< Color format: red, green 8-bit normalized unsigned integer components.
    RG8SNorm,           //!< Color format: red, green 8-bit normalized signed integer components.
    RG8UInt,            //!< Color format: red, green 8-bit unsigned integer components.
    RG8SInt,            //!< Color format: red, green 8-bit signed integer components.

    RG16UNorm,          //!< Color format: red, green 16-bit normalized unsigned interger components.
    RG16SNorm,          //!< Color format: red, green 16-bit normalized signed interger components.
    RG16UInt,           //!< Color format: red, green 16-bit unsigned interger components.
    RG16SInt,           //!< Color format: red, green 16-bit signed interger components.
    RG16Float,          //!< Color format: red, green 16-bit floating point components.

    RG32UInt,           //!< Color format: red, green 32-bit unsigned interger components.
    RG32SInt,           //!< Color format: red, green 32-bit signed interger components.
    RG32Float,          //!< Color format: red, green 32-bit floating point components.

    RG64Float,          //!< Color format: red, green 64-bit floating point components. \note Only supported with: Vulkan.

    /* --- RGB color formats --- */
    RGB8UNorm,          //!< Color format: red, green, blue 8-bit normalized unsigned integer components. \note Only supported with: OpenGL, Vulkan.
    RGB8UNorm_sRGB,     //!< Color format: red, green, blue 8-bit normalized unsigned integer components in sRGB non-linear color space. \note Only supported with: OpenGL, Vulkan.
    RGB8SNorm,          //!< Color format: red, green, blue 8-bit normalized signed integer components. \note Only supported with: OpenGL, Vulkan.
    RGB8UInt,           //!< Color format: red, green, blue 8-bit unsigned integer components. \note Only supported with: OpenGL, Vulkan.
    RGB8SInt,           //!< Color format: red, green, blue 8-bit signed integer components. \note Only supported with: OpenGL, Vulkan.

    RGB16UNorm,         //!< Color format: red, green, blue 16-bit normalized unsigned interger components. \note Only supported with: OpenGL, Vulkan.
    RGB16SNorm,         //!< Color format: red, green, blue 16-bit normalized signed interger components. \note Only supported with: OpenGL, Vulkan.
    RGB16UInt,          //!< Color format: red, green, blue 16-bit unsigned interger components. \note Only supported with: OpenGL, Vulkan.
    RGB16SInt,          //!< Color format: red, green, blue 16-bit signed interger components. \note Only supported with: OpenGL, Vulkan.
    RGB16Float,         //!< Color format: red, green, blue 16-bit floating point components. \note Only supported with: OpenGL, Vulkan.

    RGB32UInt,          //!< Color format: red, green, blue 32-bit unsigned interger components. \note As texture format only supported with: OpenGL, Vulkan, Direct3D 11, Direct3D 12.
    RGB32SInt,          //!< Color format: red, green, blue 32-bit signed interger components. \note As texture format only supported with: OpenGL, Vulkan, Direct3D 11, Direct3D 12.
    RGB32Float,         //!< Color format: red, green, blue 32-bit floating point components. \note As texture format only supported with: OpenGL, Vulkan, Direct3D 11, Direct3D 12.

    RGB64Float,         //!< Color format: red, green, blue 64-bit floating point components. \note Only supported with: Vulkan.

    /* --- RGBA color formats --- */
    RGBA8UNorm,         //!< Color format: red, green, blue, alpha 8-bit normalized unsigned integer components.
    RGBA8UNorm_sRGB,    //!< Color format: red, green, blue, alpha 8-bit normalized unsigned integer components in sRGB non-linear color space.
    RGBA8SNorm,         //!< Color format: red, green, blue, alpha 8-bit normalized signed integer components.
    RGBA8UInt,          //!< Color format: red, green, blue, alpha 8-bit unsigned integer components.
    RGBA8SInt,          //!< Color format: red, green, blue, alpha 8-bit signed integer components.

    RGBA16UNorm,        //!< Color format: red, green, blue, alpha 16-bit normalized unsigned interger components.
    RGBA16SNorm,        //!< Color format: red, green, blue, alpha 16-bit normalized signed interger components.
    RGBA16UInt,         //!< Color format: red, green, blue, alpha 16-bit unsigned interger components.
    RGBA16SInt,         //!< Color format: red, green, blue, alpha 16-bit signed interger components.
    RGBA16Float,        //!< Color format: red, green, blue, alpha 16-bit floating point components.

    RGBA32UInt,         //!< Color format: red, green, blue, alpha 32-bit unsigned interger components.
    RGBA32SInt,         //!< Color format: red, green, blue, alpha 32-bit signed interger components.
    RGBA32Float,        //!< Color format: red, green, blue, alpha 32-bit floating point components.

    RGBA64Float,        //!< Color format: red, green, blue, alpha 64-bit floating point components. \note Only supported with: Vulkan.

    /* --- BGRA color formats --- */
    BGRA8UNorm,         //!< Color format: blue, green, red, alpha 8-bit normalized unsigned integer components.
    BGRA8UNorm_sRGB,    //!< Color format: blue, green, red, alpha 8-bit normalized unsigned integer components in sRGB non-linear color space.
    BGRA8SNorm,         //!< Color format: blue, green, red, alpha 8-bit normalized signed integer components. \note Only supported with: Vulkan.
    BGRA8UInt,          //!< Color format: blue, green, red, alpha 8-bit unsigned integer components. \note Only supported with: Vulkan.
    BGRA8SInt,          //!< Color format: blue, green, red, alpha 8-bit signed integer components. \note Only supported with: Vulkan.

    /* --- Depth-stencil formats --- */
    D16UNorm,           //!< Depth-stencil format: depth 16-bit normalized unsigned integer component.
    D24UNormS8UInt,     //!< Depth-stencil format: depth 24-bit normalized unsigned integer component, and 8-bit unsigned integer stencil component.
    D32Float,           //!< Depth-stencil format: depth 32-bit floating point component.
    D32FloatS8X24UInt,  //!< Depth-stencil format: depth 32-bit floating point component, and 8-bit unsigned integer stencil components (where the remaining 24 bits are unused).

    /* --- Compressed color formats --- */
    BC1RGB,             //!< Compressed color format: RGB S3TC DXT1 with 8 bytes per 4x4 block. \note Only supported with: OpenGL, Vulkan.
    BC1RGBA,            //!< Compressed color format: RGBA S3TC DXT1 with 8 bytes per 4x4 block.
    BC2RGBA,            //!< Compressed color format: RGBA S3TC DXT3 with 16 bytes per 4x4 block.
    BC3RGBA,            //!< Compressed color format: RGBA S3TC DXT5 with 16 bytes per 4x4 block.
};

/**
\brief Image format enumeration that applies to each pixel of an image.
\see SrcImageDescriptor::format
\see DstImageDescriptor::format
\see ImageFormatSize
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
    ARGB,           //!< Four color components: Alpha, Red, Green, Blue. Old format, mainly used in Direct3D 9.
    ABGR,           //!< Four color components: Alpha, Blue, Green, Red. Old format, mainly used in Direct3D 9.

    /* Depth-stencil formats */
    Depth,          //!< Depth component.
    DepthStencil,   //!< Depth component and stencil index.

    /* Compressed formats */
    CompressedRGB,  //!< Generic compressed format with three color components: Red, Green, Blue.
    CompressedRGBA, //!< Generic compressed format with four color components: Red, Green, Blue, Alpha.
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
    Int8,       //!< 8-bit signed integer (char).
    UInt8,      //!< 8-bit unsigned integer (unsigned char).

    Int16,      //!< 16-bit signed integer (short).
    UInt16,     //!< 16-bit unsigned integer (unsigned short).

    Int32,      //!< 32-bit signed integer (int).
    UInt32,     //!< 32-bit unsigned integer (unsiged int).

    Float16,    //!< 16-bit floating-point (half).
    Float32,    //!< 32-bit floating-point (float).
    Float64,    //!< 64-bit real type (double).
};


/* ----- Structures ----- */

/**
\brief Vertex and texture format descriptor structure.
\remarks Describes all attributes of a hardware format, e.g. bit size, color components, compression etc.
\note This descriptor structure has no default initializers to fulfill the requirements of a plain-old-data (POD) structure.
\see GetFormatDesc
*/
struct FormatDescriptor
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

    //! Specifies whether the format is a color format in non-linear sRGB space (e.g. Format::RGBA8UNorm_sRGB).
    bool            sRGB;

    //! Specifies whether the format is a compressed color format (e.g. Format::BC1RGBA).
    bool            compressed;

    //! Specifies whether the format is a depth or depth-stencil format (e.g. Format::D16UNorm).
    bool            depth;

    //! Specifies whether the format is a stencil or depth-stencil format (e.g. Format::D24UNormS8UInt).
    bool            stencil;

    //! Specifies whether the format is a normalized component format (e.g. Format::R8UNorm or Format::D16UNorm).
    bool            normalized;
};


/* ----- Functions ----- */

/**
\defgroup group_format_util Hardware format utility functions.
\addtogroup group_format_util
@{
*/

/**
\brief Returns the descriptor for the specified hardware format or a zero-initialized descriptor in case of an invalid argument.
\see FormatDescriptor
\see Format
*/
LLGL_EXPORT const FormatDescriptor& GetFormatDesc(const Format format);

/**
\brief Returns true if the specified hardware format is a compressed format,
i.e. either Format::BC1RGB, Format::BC1RGBA, Format::BC2RGBA, or Format::BC3RGBA.
\see Format
*/
LLGL_EXPORT bool IsCompressedFormat(const Format format);

/**
\brief Returns true if the specified hardware format is a depth or depth-stencil format,
i.e. Format::D16UNorm, Format::D24UNormS8UInt, Format::D32Float, or Format::D32FloatS8X24UInt.
\see Format
*/
LLGL_EXPORT bool IsDepthStencilFormat(const Format format);

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
\brief Returns true if the specified hardware format is a normalized format (like Format::RGBA8UNorm, Format::R8SNorm etc.).
\remarks This does not include depth-stencil formats or compressed formats.
\see IsDepthStencilFormat
\see IsCompressedFormat
\see Format
*/
LLGL_EXPORT bool IsNormalizedFormat(const Format format);

/**
\brief Returns true if the specified hardware format is an integral format (like Format::RGBA8UInt, Format::R8SInt etc.).
\remarks This also includes all normalized formats.
\see IsNormalizedFormat
\see Format
*/
LLGL_EXPORT bool IsIntegralFormat(const Format format);

/**
\brief Returns true if the specified hardware format is a floating-point format (like Format::RGBA32Float, Format::R32Float etc.).
\remarks This does not include depth-stencil formats or compressed formats.
\see IsDepthStencilFormat
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
