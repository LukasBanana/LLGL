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


// Declaration of all hardware format descriptors
static const FormatDescriptor g_formatDescs[] =
{
//   bits  w  h  c  format                       dataType             sRGB   compr  depth  stncl  norm   pack
    {   0, 0, 0, 0, ImageFormat::R,              DataType::Int8,      false, false, false, false, false, false }, // Undefined

    /* --- Alpha channel color formats --- */
//   bits  w  h  c  format                       dataType             sRGB   compr  depth  stncl  norm   pack
    {   8, 1, 1, 1, ImageFormat::Alpha,          DataType::UInt8,     false, false, false, false, true,  false }, // A8UNorm

    /* --- Red channel color formats --- */
//   bits  w  h  c  format                       dataType             sRGB   compr  depth  stncl  norm   pack
    {   8, 1, 1, 1, ImageFormat::R,              DataType::UInt8,     false, false, false, false, true,  false }, // R8UNorm
    {   8, 1, 1, 1, ImageFormat::R,              DataType::Int8,      false, false, false, false, true,  false }, // R8SNorm
    {   8, 1, 1, 1, ImageFormat::R,              DataType::UInt8,     false, false, false, false, false, false }, // R8UInt
    {   8, 1, 1, 1, ImageFormat::R,              DataType::Int8,      false, false, false, false, false, false }, // R8SInt

    {  16, 1, 1, 1, ImageFormat::R,              DataType::UInt16,    false, false, false, false, true,  false }, // R16UNorm
    {  16, 1, 1, 1, ImageFormat::R,              DataType::Int16,     false, false, false, false, true,  false }, // R16SNorm
    {  16, 1, 1, 1, ImageFormat::R,              DataType::UInt16,    false, false, false, false, false, false }, // R16UInt
    {  16, 1, 1, 1, ImageFormat::R,              DataType::Int16,     false, false, false, false, false, false }, // R16SInt
    {  16, 1, 1, 1, ImageFormat::R,              DataType::Float16,   false, false, false, false, false, false }, // R16Float

    {  32, 1, 1, 1, ImageFormat::R,              DataType::UInt32,    false, false, false, false, false, false }, // R32UInt
    {  32, 1, 1, 1, ImageFormat::R,              DataType::Int32,     false, false, false, false, false, false }, // R32SInt
    {  32, 1, 1, 1, ImageFormat::R,              DataType::Float32,   false, false, false, false, false, false }, // R32Float

    {  64, 1, 1, 1, ImageFormat::R,              DataType::Float64,   false, false, false, false, false, false }, // R64Float

    /* --- RG color formats --- */
//   bits  w  h  c  format                       dataType             sRGB   compr  depth  stncl  norm   pack
    {  16, 1, 1, 2, ImageFormat::RG,             DataType::UInt8,     false, false, false, false, true,  false }, // RG8UNorm
    {  16, 1, 1, 2, ImageFormat::RG,             DataType::Int8,      false, false, false, false, true,  false }, // RG8SNorm
    {  16, 1, 1, 2, ImageFormat::RG,             DataType::UInt8,     false, false, false, false, false, false }, // RG8UInt
    {  16, 1, 1, 2, ImageFormat::RG,             DataType::Int8,      false, false, false, false, false, false }, // RG8SInt

    {  32, 1, 1, 2, ImageFormat::RG,             DataType::UInt16,    false, false, false, false, true,  false }, // RG16UNorm
    {  32, 1, 1, 2, ImageFormat::RG,             DataType::Int16,     false, false, false, false, true,  false }, // RG16SNorm
    {  32, 1, 1, 2, ImageFormat::RG,             DataType::UInt16,    false, false, false, false, false, false }, // RG16UInt
    {  32, 1, 1, 2, ImageFormat::RG,             DataType::Int16,     false, false, false, false, false, false }, // RG16SInt
    {  32, 1, 1, 2, ImageFormat::RG,             DataType::Float16,   false, false, false, false, false, false }, // RG16Float

    {  64, 1, 1, 2, ImageFormat::RG,             DataType::UInt32,    false, false, false, false, false, false }, // RG32UInt
    {  64, 1, 1, 2, ImageFormat::RG,             DataType::Int32,     false, false, false, false, false, false }, // RG32SInt
    {  64, 1, 1, 2, ImageFormat::RG,             DataType::Float32,   false, false, false, false, false, false }, // RG32Float

    { 128, 1, 1, 2, ImageFormat::RG,             DataType::Float64,   false, false, false, false, false, false }, // RG64Float

    /* --- RGB color formats --- */
//   bits  w  h  c  format                       dataType             sRGB   compr  depth  stncl  norm   pack
    {  24, 1, 1, 3, ImageFormat::RGB,            DataType::UInt8,     false, false, false, false, true,  false }, // RGB8UNorm
    {  24, 1, 1, 3, ImageFormat::RGB,            DataType::UInt8,     true,  false, false, false, true,  false }, // RGB8UNorm_sRGB
    {  24, 1, 1, 3, ImageFormat::RGB,            DataType::Int8,      false, false, false, false, true,  false }, // RGB8SNorm
    {  24, 1, 1, 3, ImageFormat::RGB,            DataType::UInt8,     false, false, false, false, false, false }, // RGB8UInt
    {  24, 1, 1, 3, ImageFormat::RGB,            DataType::Int8,      false, false, false, false, false, false }, // RGB8SInt

    {  48, 1, 1, 3, ImageFormat::RGB,            DataType::UInt16,    false, false, false, false, true,  false }, // RGB16UNorm
    {  48, 1, 1, 3, ImageFormat::RGB,            DataType::Int16,     false, false, false, false, true,  false }, // RGB16SNorm
    {  48, 1, 1, 3, ImageFormat::RGB,            DataType::UInt16,    false, false, false, false, false, false }, // RGB16UInt
    {  48, 1, 1, 3, ImageFormat::RGB,            DataType::Int16,     false, false, false, false, false, false }, // RGB16SInt
    {  48, 1, 1, 3, ImageFormat::RGB,            DataType::Float16,   false, false, false, false, false, false }, // RGB16Float

    {  96, 1, 1, 3, ImageFormat::RGB,            DataType::UInt32,    false, false, false, false, false, false }, // RGB32UInt
    {  96, 1, 1, 3, ImageFormat::RGB,            DataType::Int32,     false, false, false, false, false, false }, // RGB32SInt
    {  96, 1, 1, 3, ImageFormat::RGB,            DataType::Float32,   false, false, false, false, false, false }, // RGB32Float

    { 192, 1, 1, 3, ImageFormat::RGB,            DataType::Float64,   false, false, false, false, false, false }, // RGB64Float

    /* --- RGBA color formats --- */
//   bits  w  h  c  format                       dataType             sRGB   compr  depth  stncl  norm   pack
    {  32, 1, 1, 4, ImageFormat::RGBA,           DataType::UInt8,     false, false, false, false, true,  false }, // RGBA8UNorm
    {  32, 1, 1, 4, ImageFormat::RGBA,           DataType::UInt8,     true,  false, false, false, true,  false }, // RGBA8UNorm_sRGB
    {  32, 1, 1, 4, ImageFormat::RGBA,           DataType::Int8,      false, false, false, false, true,  false }, // RGBA8SNorm
    {  32, 1, 1, 4, ImageFormat::RGBA,           DataType::UInt8,     false, false, false, false, false, false }, // RGBA8UInt
    {  32, 1, 1, 4, ImageFormat::RGBA,           DataType::Int8,      false, false, false, false, false, false }, // RGBA8SInt

    {  64, 1, 1, 4, ImageFormat::RGBA,           DataType::UInt16,    false, false, false, false, true,  false }, // RGBA16UNorm
    {  64, 1, 1, 4, ImageFormat::RGBA,           DataType::Int16,     false, false, false, false, true,  false }, // RGBA16SNorm
    {  64, 1, 1, 4, ImageFormat::RGBA,           DataType::UInt16,    false, false, false, false, false, false }, // RGBA16UInt
    {  64, 1, 1, 4, ImageFormat::RGBA,           DataType::Int16,     false, false, false, false, false, false }, // RGBA16SInt
    {  64, 1, 1, 4, ImageFormat::RGBA,           DataType::Float16,   false, false, false, false, false, false }, // RGBA16Float

    { 128, 1, 1, 4, ImageFormat::RGBA,           DataType::UInt32,    false, false, false, false, false, false }, // RGBA32UInt
    { 128, 1, 1, 4, ImageFormat::RGBA,           DataType::Int32,     false, false, false, false, false, false }, // RGBA32SInt
    { 128, 1, 1, 4, ImageFormat::RGBA,           DataType::Float32,   false, false, false, false, false, false }, // RGBA32Float

    { 256, 1, 1, 4, ImageFormat::RGBA,           DataType::Float64,   false, false, false, false, false, false }, // RGBA64Float

    /* --- BGRA color formats --- */
//   bits  w  h  c  format                       dataType             sRGB   compr  depth  stncl  norm   pack
    {  32, 1, 1, 4, ImageFormat::BGRA,           DataType::UInt8,     false, false, false, false, true,  false }, // BGRA8UNorm
    {  32, 1, 1, 4, ImageFormat::BGRA,           DataType::UInt8,     true,  false, false, false, true,  false }, // BGRA8UNorm_sRGB
    {  32, 1, 1, 4, ImageFormat::BGRA,           DataType::Int8,      false, false, false, false, true,  false }, // BGRA8SNorm
    {  32, 1, 1, 4, ImageFormat::BGRA,           DataType::UInt8,     false, false, false, false, false, false }, // BGRA8UInt
    {  32, 1, 1, 4, ImageFormat::BGRA,           DataType::Int8,      false, false, false, false, false, false }, // BGRA8SInt

    /* --- Packed formats --- */
//   bits  w  h  c  format                       dataType             sRGB   compr  depth  stncl  norm   pack
    {  32, 1, 1, 4, ImageFormat::RGBA,           DataType::Undefined, false, false, false, false, true,  true  }, // RGB10A2UNorm
    {  32, 1, 1, 4, ImageFormat::RGBA,           DataType::Undefined, false, false, false, false, false, true  }, // RGB10A2UInt
    {  32, 1, 1, 3, ImageFormat::RGB,            DataType::Undefined, false, false, false, false, false, true  }, // RG11B10Float
    {  32, 1, 1, 3, ImageFormat::RGB,            DataType::Undefined, false, false, false, false, false, true  }, // RGB9E5Float

    /* --- Depth-stencil formats --- */
//   bits  w  h  c  format                       dataType             sRGB   compr  depth  stncl  norm   pack
    {  16, 1, 1, 1, ImageFormat::Depth,          DataType::UInt16,    false, false, true,  false, true,  false }, // D16UNorm
    {  32, 1, 1, 2, ImageFormat::DepthStencil,   DataType::UInt16,    false, false, true,  true,  true,  false }, // D24UNormS8UInt
    {  32, 1, 1, 1, ImageFormat::Depth,          DataType::Float32,   false, false, true,  false, false, false }, // D32Float
    {  64, 1, 1, 2, ImageFormat::DepthStencil,   DataType::Float32,   false, false, true,  true,  false, false }, // D32FloatS8X24UInt
  //{   8, 1, 1, 1, ImageFormat::Stencil,        DataType::UInt8,     false, false, false, true,  false, false }, // S8UInt

    /* --- Compressed color formats --- */
//   bits  w  h  c  format                       dataType             sRGB   compr  depth  stncl  norm   pack
    {  64, 4, 4, 3, ImageFormat::CompressedRGB,  DataType::Int8,      false, true,  false, false, false, false }, // BC1RGB
    {  64, 4, 4, 4, ImageFormat::CompressedRGBA, DataType::Int8,      false, true,  false, false, false, false }, // BC1RGBA
  //{  64, 4, 4, 4, ImageFormat::CompressedRGBA, DataType::Int8,      true,  true,  false, false, false, false }, // BC1RGBA_sRGB
    { 128, 4, 4, 4, ImageFormat::CompressedRGBA, DataType::Int8,      false, true,  false, false, false, false }, // BC2RGBA
  //{ 128, 4, 4, 4, ImageFormat::CompressedRGBA, DataType::Int8,      true,  true,  false, false, false, false }, // BC2RGBA_sRGB
    { 128, 4, 4, 4, ImageFormat::CompressedRGBA, DataType::Int8,      false, true,  false, false, false, false }, // BC3RGBA
  //{ 128, 4, 4, 4, ImageFormat::CompressedRGBA, DataType::Int8,      true,  true,  false, false, false, false }, // BC3RGBA_sRGB
};

LLGL_EXPORT const FormatDescriptor& GetFormatDesc(const Format format)
{
    auto idx = static_cast<std::size_t>(format);
    if (idx < sizeof(g_formatDescs)/sizeof(g_formatDescs[0]))
        return g_formatDescs[idx];
    else
        return g_formatDescs[0];
}

LLGL_EXPORT bool IsCompressedFormat(const Format format)
{
    return GetFormatDesc(format).compressed;
}

LLGL_EXPORT bool IsDepthStencilFormat(const Format format)
{
    const auto& formatDesc = GetFormatDesc(format);
    return (formatDesc.depth || formatDesc.stencil);
}

LLGL_EXPORT bool IsDepthFormat(const Format format)
{
    return GetFormatDesc(format).depth;
}

LLGL_EXPORT bool IsStencilFormat(const Format format)
{
    return GetFormatDesc(format).stencil;
}

LLGL_EXPORT bool IsNormalizedFormat(const Format format)
{
    return GetFormatDesc(format).normalized;
}

LLGL_EXPORT bool IsIntegralFormat(const Format format)
{
    if (format >= Format::R8UNorm && format <= Format::BGRA8SInt)
        return !IsFloatFormat(format);
    else
        return false;
}

LLGL_EXPORT bool IsFloatFormat(const Format format)
{
    switch (format)
    {
        case Format::R16Float:
        case Format::R64Float:
        case Format::R32Float:
        case Format::RG16Float:
        case Format::RG32Float:
        case Format::RG64Float:
        case Format::RGB16Float:
        case Format::RGB32Float:
        case Format::RGB64Float:
        case Format::RGBA16Float:
        case Format::RGBA32Float:
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
