/*
 * CsFormat.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#pragma once


namespace SharpLLGL
{


/// <summary>Hardware vector and pixel format enumeration.</summary>
public enum class Format
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

public enum class DataType
{
    Undefined,

    Int8,
    UInt8,

    Int16,
    UInt16,

    Int32,
    UInt32,

    Float16,
    Float32,
    Float64,
};


} // /namespace SharpLLGL



// ================================================================================
