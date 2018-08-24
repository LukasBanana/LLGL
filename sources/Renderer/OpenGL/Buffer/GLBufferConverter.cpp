/*
 * GLBufferConverter.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLBufferConverter.h"
#include "../../../Core/Float16Compressor.h"
#include <algorithm>
#include <limits.h>


namespace LLGL
{

namespace GLBufferConverter
{


/*
 * Internal functions
 */

// Reads the variant to a 64-bit floating-point value
static void ReadVariant(const FormatData& variant, const DataType baseType, int component, bool normalized, double& value)
{
    switch (baseType)
    {
        case DataType::Int8:
            value = static_cast<double>(variant.int8[component]);
            break;

        case DataType::UInt8:
            if (normalized)
                value = static_cast<double>(variant.uint8[component]) / static_cast<double>(UCHAR_MAX);
            else
                value = static_cast<double>(variant.uint8[component]);
            break;

        case DataType::Int16:
            value = static_cast<double>(variant.int16[component]);
            break;

        case DataType::UInt16:
            if (normalized)
                value = static_cast<double>(variant.uint16[component]) / static_cast<double>(USHRT_MAX);
            else
                value = static_cast<double>(variant.uint16[component]);
            break;

        case DataType::Int32:
            value = static_cast<double>(variant.int32[component]);
            break;

        case DataType::UInt32:
            value = static_cast<double>(variant.uint32[component]);
            break;

        case DataType::Float16:
            value = static_cast<double>(DecompressFloat16(variant.uint16[component]));
            break;

        case DataType::Float32:
            value = static_cast<double>(variant.real32[component]);
            break;

        case DataType::Float64:
            break;
    }
}

// Write the variant from a 64-bit floating-point value
static void WriteVariant(FormatData& variant, const DataType baseType, int component, bool normalized, double value)
{
    switch (baseType)
    {
        case DataType::Int8:
            variant.int8[component] = static_cast<std::int8_t>(value);
            break;

        case DataType::UInt8:
            if (normalized)
                variant.uint8[component] = static_cast<std::uint8_t>(value * static_cast<double>(UCHAR_MAX));
            else
                variant.uint8[component] = static_cast<std::uint8_t>(value);
            break;

        case DataType::Int16:
            variant.int16[component] = static_cast<std::int16_t>(value);
            break;

        case DataType::UInt16:
            if (normalized)
                variant.uint16[component] = static_cast<std::uint16_t>(value * static_cast<double>(USHRT_MAX));
            else
                variant.uint16[component] = static_cast<std::uint16_t>(value);
            break;

        case DataType::Int32:
            variant.int32[component] = static_cast<std::int32_t>(value);
            break;

        case DataType::UInt32:
            variant.uint32[component] = static_cast<std::uint32_t>(value);
            break;

        case DataType::Float16:
            variant.uint16[component] = CompressFloat16(static_cast<float>(value));
            break;

        case DataType::Float32:
            variant.real32[component] = static_cast<float>(value);
            break;

        case DataType::Float64:
            break;
    }
}

// Reads the variant to a 32-bit unsigned interger value
static void ReadVariant(const FormatData& variant, const DataType baseType, int component, bool normalized, std::uint32_t& value)
{
    switch (baseType)
    {
        case DataType::Int8:
            value = static_cast<std::uint32_t>(variant.int8[component]);
            break;

        case DataType::UInt8:
            value = static_cast<std::uint32_t>(variant.uint8[component]);
            break;

        case DataType::Int16:
            value = static_cast<std::uint32_t>(variant.int16[component]);
            break;

        case DataType::UInt16:
            value = static_cast<std::uint32_t>(variant.uint16[component]);
            break;

        case DataType::Int32:
            value = static_cast<std::uint32_t>(variant.int32[component]);
            break;

        case DataType::UInt32:
            value = variant.uint32[component];
            break;

        default:
            break;
    }
}

// Write the variant from a 32-bit unsigned interger value
static void WriteVariant(FormatData& variant, const DataType baseType, int component, std::uint32_t value)
{
    switch (baseType)
    {
        case DataType::Int8:
            variant.int8[component] = static_cast<std::int8_t>(value);
            break;

        case DataType::UInt8:
            variant.uint8[component] = static_cast<std::uint8_t>(value);
            break;

        case DataType::Int16:
            variant.int16[component] = static_cast<std::int16_t>(value);
            break;

        case DataType::UInt16:
            variant.uint16[component] = static_cast<std::uint16_t>(value);
            break;

        case DataType::Int32:
            variant.int32[component] = static_cast<std::int32_t>(value);
            break;

        case DataType::UInt32:
            variant.uint32[component] = static_cast<std::uint32_t>(value);
            break;

        default:
            break;
    }
}


/*
 * Global functions
 */

FormatDataDescriptor GetFormatDataDesc(GLenum format)
{
    switch (format)
    {
        case GL_R8:         return { DataType::UInt8  , 1, true  };
        case GL_R16:        return { DataType::UInt16 , 1, true  };
        case GL_R16F:       return { DataType::Float16, 1, false };
        case GL_R32F:       return { DataType::Float32, 1, false };
        case GL_R8I:        return { DataType::Int8   , 1, false };
        case GL_R16I:       return { DataType::Int16  , 1, false };
        case GL_R32I:       return { DataType::Int32  , 1, false };
        case GL_R8UI:       return { DataType::UInt8  , 1, false };
        case GL_R16UI:      return { DataType::UInt16 , 1, false };
        case GL_R32UI:      return { DataType::UInt32 , 1, false };
        case GL_RG8:        return { DataType::UInt8  , 2, true  };
        case GL_RG16:       return { DataType::UInt16 , 2, true  };
        case GL_RG16F:      return { DataType::Float16, 2, false };
        case GL_RG32F:      return { DataType::Float32, 2, false };
        case GL_RG8I:       return { DataType::Int8   , 2, false };
        case GL_RG16I:      return { DataType::Int16  , 2, false };
        case GL_RG32I:      return { DataType::Int32  , 2, false };
        case GL_RG8UI:      return { DataType::UInt8  , 2, false };
        case GL_RG16UI:     return { DataType::UInt16 , 2, false };
        case GL_RG32UI:     return { DataType::UInt32 , 2, false };
        case GL_RGB32F:     return { DataType::Float32, 3, false };
        case GL_RGB32I:     return { DataType::Int32  , 3, false };
        case GL_RGB32UI:    return { DataType::UInt32 , 3, false };
        case GL_RGBA8:      return { DataType::UInt8  , 4, true  };
        case GL_RGBA16:     return { DataType::UInt16 , 4, true  };
        case GL_RGBA16F:    return { DataType::Float16, 4, false };
        case GL_RGBA32F:    return { DataType::Float32, 4, false };
        case GL_RGBA8I:     return { DataType::Int8   , 4, false };
        case GL_RGBA16I:    return { DataType::Int16  , 4, false };
        case GL_RGBA32I:    return { DataType::Int32  , 4, false };
        case GL_RGBA8UI:    return { DataType::UInt8  , 4, false };
        case GL_RGBA16UI:   return { DataType::UInt16 , 4, false };
        case GL_RGBA32UI:   return { DataType::UInt32 , 4, false };
    }
    return { DataType::Int8, 0, false };
}

std::size_t ConvertFormatData(
    FormatData&                 dst,
    const FormatDataDescriptor& dstDesc,
    const FormatData&           src,
    const FormatDataDescriptor& srcDesc)
{
    /* Check if conversion is possible and necessary */
    if ( dstDesc.components >= 1 && dstDesc.components <= 4 &&
         srcDesc.components >= 1 && srcDesc.components <= 4 &&
         ( dstDesc.baseType != srcDesc.baseType || dstDesc.components != srcDesc.components || dstDesc.normalized != srcDesc.normalized ) )
    {
        /* Check if conversion should take place with a floating-point intermediate buffer */
        if (IsFloatDataType(dstDesc.baseType) || IsFloatDataType(srcDesc.baseType) || dstDesc.normalized || srcDesc.normalized)
        {
            /* Initialize default values for intermediate format data */
            double intermediate[4] = { 0.0, 0.0, 0.0, 1.0 };

            /* First convert into 64-bit floats to reduce data loss */
            for (int i = 0; i < srcDesc.components; ++i)
                ReadVariant(src, srcDesc.baseType, i, srcDesc.normalized, intermediate[i]);

            /* Convert intermediate data into output */
            for (int i = 0; i < srcDesc.components; ++i)
                WriteVariant(dst, dstDesc.baseType, i, dstDesc.normalized, intermediate[i]);
        }
        else
        {
            /* Initialize default values for intermediate format data */
            std::uint32_t intermediate[4] = { 0, 0, 0, UINT32_MAX };

            /* First convert into 32-bit integers */
            for (int i = 0; i < srcDesc.components; ++i)
                ReadVariant(src, srcDesc.baseType, i, srcDesc.normalized, intermediate[i]);

            /* Convert intermediate data into output */
            for (int i = 0; i < srcDesc.components; ++i)
                WriteVariant(dst, dstDesc.baseType, i, dstDesc.normalized, intermediate[i]);
        }
    }
    else
    {
        /* Copy source to destination */
        dst = src;
    }

    /* Return destination format size */
    return (static_cast<std::size_t>(dstDesc.components) * DataTypeSize(dstDesc.baseType));
}


} // /namespace GLBufferConverter

} // /namespace LLGL



// ================================================================================
