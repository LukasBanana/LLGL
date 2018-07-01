/*
 * Format.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/Format.h>
#include <tuple>


namespace LLGL
{


LLGL_EXPORT std::uint32_t FormatBitSize(const Format format)
{
    switch (format)
    {
        /* --- Color formats --- */
        case Format::R8UNorm:           return 8;
        case Format::R8SNorm:           return 8;

        case Format::R16UNorm:          return 16;
        case Format::R16SNorm:          return 16;
        case Format::R16Float:          return 16;

        case Format::R32UInt:           return 32;
        case Format::R32SInt:           return 32;
        case Format::R32Float:          return 32;

        case Format::RG8UNorm:          return 16;
        case Format::RG8SNorm:          return 16;

        case Format::RG16UNorm:         return 32;
        case Format::RG16SNorm:         return 32;
        case Format::RG16Float:         return 32;

        case Format::RG32UInt:          return 64;
        case Format::RG32SInt:          return 64;
        case Format::RG32Float:         return 64;

        case Format::RGB8UNorm:         return 24;
        case Format::RGB8SNorm:         return 24;

        case Format::RGB16UNorm:        return 48;
        case Format::RGB16SNorm:        return 48;
        case Format::RGB16Float:        return 48;

        case Format::RGB32UInt:         return 96;
        case Format::RGB32SInt:         return 96;
        case Format::RGB32Float:        return 96;

        case Format::RGBA8UNorm:        return 32;
        case Format::RGBA8SNorm:        return 32;

        case Format::RGBA16UNorm:       return 64;
        case Format::RGBA16SNorm:       return 64;
        case Format::RGBA16Float:       return 64;

        case Format::RGBA32UInt:        return 128;
        case Format::RGBA32SInt:        return 128;
        case Format::RGBA32Float:       return 128;

        /* --- Extended color formats --- */
        case Format::R64Float:          return 64;
        case Format::RG64Float:         return 128;
        case Format::RGB64Float:        return 192;
        case Format::RGBA64Float:       return 256;

        /* --- Depth-stencil formats --- */
        case Format::D16UNorm:          return 16;  // 16-bit depth
        case Format::D32Float:          return 32;  // 32-bit depth
        case Format::D24UNormS8UInt:    return 32;  // 24-bit depth, 8-bit stencil
        case Format::D32FloatS8X24UInt: return 64;  // 32-bit depth, 8-bit stencil, 24-bit unused

        /* --- Compressed color formats --- */
        case Format::BC1RGB:            return 4;   // 64-bit per 4x4 block
        case Format::BC1RGBA:           return 4;   // 64-bit per 4x4 block
        case Format::BC2RGBA:           return 8;   // 128-bit per 4x4 block
        case Format::BC3RGBA:           return 8;   // 128-bit per 4x4 block

        default:                        return 0;
    }
}

static std::tuple<DataType, std::uint32_t> SplitFormatPrimary(const Format format)
{
    using T = std::tuple<DataType, std::uint32_t>;
    switch (format)
    {
        case Format::Undefined:         break;

        /* --- Color formats --- */
        case Format::R8UNorm:           return T{ DataType::UInt8,   1 };
        case Format::R8SNorm:           return T{ DataType::Int8,    1 };
        case Format::R8UInt:            return T{ DataType::UInt8,   1 };
        case Format::R8SInt:            return T{ DataType::Int8,    1 };

        case Format::R16UNorm:          return T{ DataType::UInt16,  1 };
        case Format::R16SNorm:          return T{ DataType::Int16,   1 };
        case Format::R16UInt:           return T{ DataType::UInt16,  1 };
        case Format::R16SInt:           return T{ DataType::Int16,   1 };
        case Format::R16Float:          return T{ DataType::Float16, 1 };

        case Format::R32UInt:           return T{ DataType::UInt32,  2 };
        case Format::R32SInt:           return T{ DataType::Int32,   2 };
        case Format::R32Float:          return T{ DataType::Float32, 2 };

        case Format::RG8UNorm:          return T{ DataType::UInt8,   2 };
        case Format::RG8SNorm:          return T{ DataType::Int8,    2 };
        case Format::RG8UInt:           return T{ DataType::UInt8,   2 };
        case Format::RG8SInt:           return T{ DataType::Int8,    2 };

        case Format::RG16UNorm:         return T{ DataType::UInt16,  2 };
        case Format::RG16SNorm:         return T{ DataType::Int16,   2 };
        case Format::RG16UInt:          return T{ DataType::UInt16,  2 };
        case Format::RG16SInt:          return T{ DataType::Int16,   2 };
        case Format::RG16Float:         return T{ DataType::Float16, 2 };

        case Format::RG32UInt:          return T{ DataType::UInt32,  2 };
        case Format::RG32SInt:          return T{ DataType::Int32,   2 };
        case Format::RG32Float:         return T{ DataType::Float32, 2 };

        case Format::RGB8UNorm:         return T{ DataType::UInt8,   3 };
        case Format::RGB8SNorm:         return T{ DataType::Int8,    3 };
        case Format::RGB8UInt:          return T{ DataType::UInt8,   3 };
        case Format::RGB8SInt:          return T{ DataType::Int8,    3 };

        case Format::RGB16UNorm:        return T{ DataType::UInt16,  3 };
        case Format::RGB16SNorm:        return T{ DataType::Int16,   3 };
        case Format::RGB16UInt:         return T{ DataType::UInt16,  3 };
        case Format::RGB16SInt:         return T{ DataType::Int16,   3 };
        case Format::RGB16Float:        return T{ DataType::Float16, 3 };

        case Format::RGB32UInt:         return T{ DataType::UInt32,  3 };
        case Format::RGB32SInt:         return T{ DataType::Int32,   3 };
        case Format::RGB32Float:        return T{ DataType::Float32, 3 };

        case Format::RGBA8UNorm:        return T{ DataType::UInt8,   4 };
        case Format::RGBA8SNorm:        return T{ DataType::Int8,    4 };
        case Format::RGBA8UInt:         return T{ DataType::UInt8,   4 };
        case Format::RGBA8SInt:         return T{ DataType::Int8,    4 };

        case Format::RGBA16UNorm:       return T{ DataType::UInt16,  4 };
        case Format::RGBA16SNorm:       return T{ DataType::Int16,   4 };
        case Format::RGBA16UInt:        return T{ DataType::UInt16,  4 };
        case Format::RGBA16SInt:        return T{ DataType::Int16,   4 };
        case Format::RGBA16Float:       return T{ DataType::Float16, 4 };

        case Format::RGBA32UInt:        return T{ DataType::UInt32,  4 };
        case Format::RGBA32SInt:        return T{ DataType::Int32,   4 };
        case Format::RGBA32Float:       return T{ DataType::Float32, 4 };

        /* --- Extended color formats --- */
        case Format::R64Float:          return T{ DataType::Float64, 1 };
        case Format::RG64Float:         return T{ DataType::Float64, 2 };
        case Format::RGB64Float:        return T{ DataType::Float64, 3 };
        case Format::RGBA64Float:       return T{ DataType::Float64, 4 };

        /* --- Depth-stencil formats --- */
        case Format::D16UNorm:          break;
        case Format::D32Float:          break;
        case Format::D24UNormS8UInt:    break;
        case Format::D32FloatS8X24UInt: break;

        /* --- Compressed color formats --- */
        case Format::BC1RGB:            break;
        case Format::BC1RGBA:           break;
        case Format::BC2RGBA:           break;
        case Format::BC3RGBA:           break;
    }

    /* Return an invalid image format */
    return T{ DataType::UInt8, 0 };
}

LLGL_EXPORT bool SplitFormat(const Format format, DataType& dataType, std::uint32_t& components)
{
    /* Find suitable format and check for invalid output */
    DataType dataTypeTmp;
    std::uint32_t componentsTmp;

    std::tie(dataTypeTmp, componentsTmp) = SplitFormatPrimary(format);

    if (componentsTmp > 0)
    {
        dataType    = dataTypeTmp;
        components  = componentsTmp;
        return true;
    }

    return false;
}

LLGL_EXPORT bool IsCompressedFormat(const Format format)
{
    return (format >= Format::BC1RGB && format <= Format::BC3RGBA);
}

LLGL_EXPORT bool IsDepthStencilFormat(const Format format)
{
    return (format >= Format::D16UNorm && format <= Format::D32FloatS8X24UInt);
}

LLGL_EXPORT bool IsNormalizedFormat(const Format format)
{
    switch (format)
    {
        case Format::R8UNorm:
        case Format::R8SNorm:
        case Format::R16UNorm:
        case Format::R16SNorm:
        case Format::RG8UNorm:
        case Format::RG8SNorm:
        case Format::RG16UNorm:
        case Format::RG16SNorm:
        case Format::RGB8UNorm:
        case Format::RGB8SNorm:
        case Format::RGB16UNorm:
        case Format::RGB16SNorm:
        case Format::RGBA8UNorm:
        case Format::RGBA8SNorm:
        case Format::RGBA16UNorm:
        case Format::RGBA16SNorm:
            return true;
        default:
            return false;
    }
}

LLGL_EXPORT bool IsIntegralFormat(const Format format)
{
    if (format >= Format::R8UNorm && format <= Format::RGBA32SInt)
        return !IsFloatFormat(format);
    else
        return false;
}

LLGL_EXPORT bool IsFloatFormat(const Format format)
{
    switch (format)
    {
        case Format::R16Float:
        case Format::R32Float:
        case Format::RG16Float:
        case Format::RG32Float:
        case Format::RGB16Float:
        case Format::RGB32Float:
        case Format::RGBA16Float:
        case Format::RGBA32Float:
        case Format::R64Float:
        case Format::RG64Float:
        case Format::RGB64Float:
        case Format::RGBA64Float:
            return true;
        default:
            return false;
    }
}

LLGL_EXPORT std::uint32_t DataTypeSize(const DataType dataType)
{
    switch (dataType)
    {
        case DataType::Int8:    return 1;
        case DataType::UInt8:   return 1;
        case DataType::Int16:   return 2;
        case DataType::UInt16:  return 2;
        case DataType::Int32:   return 4;
        case DataType::UInt32:  return 4;
        case DataType::Float16: return 2;
        case DataType::Float32: return 4;
        case DataType::Float64: return 8;
    }
    return 0;
}

LLGL_EXPORT bool IsIntDataType(const DataType dataType)
{
    return (dataType == DataType::Int8 || dataType == DataType::Int16 || dataType == DataType::Int32);
}

LLGL_EXPORT bool IsUIntDataType(const DataType dataType)
{
    return (dataType == DataType::UInt8 || dataType == DataType::UInt16 || dataType == DataType::UInt32);
}

LLGL_EXPORT bool IsFloatDataType(const DataType dataType)
{
    return (dataType >= DataType::Float16 && dataType <= DataType::Float64);
}


} // /namespace LLGL



// ================================================================================
