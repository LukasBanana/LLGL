/*
 * CsFormat.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#pragma once


namespace LHermanns
{

namespace LLGL
{


/// <summary>Hardware vector and pixel format enumeration.</summary>
enum class Format
{
    Undefined,

    /* --- Color formats --- */
    R8UNorm,
    R8SNorm,
    R8UInt,
    R8SInt,

    R16UNorm,
    R16SNorm,
    R16UInt,
    R16SInt,
    R16Float,

    R32UInt,
    R32SInt,
    R32Float,

    RG8UNorm,
    RG8SNorm,
    RG8UInt,
    RG8SInt,

    RG16UNorm,
    RG16SNorm,
    RG16UInt,
    RG16SInt,
    RG16Float,

    RG32UInt,
    RG32SInt,
    RG32Float,

    RGB8UNorm,
    RGB8SNorm,
    RGB8UInt,
    RGB8SInt,

    RGB16UNorm,
    RGB16SNorm,
    RGB16UInt,
    RGB16SInt,
    RGB16Float,

    RGB32UInt,
    RGB32SInt,
    RGB32Float,

    RGBA8UNorm,
    RGBA8SNorm,
    RGBA8UInt,
    RGBA8SInt,

    RGBA16UNorm,
    RGBA16SNorm,
    RGBA16UInt,
    RGBA16SInt,
    RGBA16Float,

    RGBA32UInt,
    RGBA32SInt,
    RGBA32Float,

    /* --- Extended color formats --- */
    R64Float,
    RG64Float,
    RGB64Float,
    RGBA64Float,

    /* --- Reversed color formats --- */
    BGRA8UNorm,
    BGRA8SNorm,
    BGRA8UInt,
    BGRA8SInt,
    BGRA8sRGB,

    /* --- Depth-stencil formats --- */
    D16UNorm,
    D24UNormS8UInt,
    D32Float,
    D32FloatS8X24UInt,

    /* --- Compressed color formats --- */
    BC1RGB,
    BC1RGBA,
    BC2RGBA,
    BC3RGBA,
};

public enum class DataType
{
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


} // /namespace LLGL

} // /namespace LHermanns



// ================================================================================
