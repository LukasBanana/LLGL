/*
 * MTTypes.mm
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "MTTypes.h"
#include <stdexcept>
#include <string>


namespace LLGL
{

namespace MTTypes
{


/* ----- Map functions ----- */

[[noreturn]]
void MapFailed(const std::string& typeName, const std::string& mtlTypeName)
{
    throw std::invalid_argument("failed to map <LLGL::" + typeName + "> to <" + mtlTypeName + "> Metal parameter");
}

MTLDataType Map(const DataType dataType)
{
    switch (dataType)
    {
        case DataType::Int8:    return MTLDataTypeChar;
        case DataType::UInt8:   return MTLDataTypeUChar;

        case DataType::Int16:   return MTLDataTypeShort;
        case DataType::UInt16:  return MTLDataTypeUShort;

        case DataType::Int32:   return MTLDataTypeInt;
        case DataType::UInt32:  return MTLDataTypeUInt;

        case DataType::Float:   return MTLDataTypeFloat;
        case DataType::Double:  break;
    }
    MapFailed("DataType", "MTLDataType");
}

MTLDataType Map(const VectorType vectorType)
{
    switch (vectorType)
    {
    	case VectorType::Float:     return MTLDataTypeFloat;
        case VectorType::Float2:    return MTLDataTypeFloat2;
        case VectorType::Float3:    return MTLDataTypeFloat3;
        case VectorType::Float4:    return MTLDataTypeFloat4;
        case VectorType::Double:    break;
        case VectorType::Double2:   break;
        case VectorType::Double3:   break;
        case VectorType::Double4:   break;
        case VectorType::Int:       return MTLDataTypeInt;
        case VectorType::Int2:      return MTLDataTypeInt2;
        case VectorType::Int3:      return MTLDataTypeInt3;
        case VectorType::Int4:      return MTLDataTypeInt4;
        case VectorType::UInt:      return MTLDataTypeUInt;
        case VectorType::UInt2:     return MTLDataTypeUInt2;
        case VectorType::UInt3:     return MTLDataTypeUInt3;
        case VectorType::UInt4:     return MTLDataTypeUInt4;
    }
    MapFailed("VectorType", "MTLDataType");
}

MTLPixelFormat Map(const TextureFormat textureFormat)
{
    switch (textureFormat)
    {
        case TextureFormat::Unknown:        break;

        /* --- Color formats --- */
        case TextureFormat::R8:             return MTLPixelFormatR8Unorm;
        case TextureFormat::R8Sgn:          return MTLPixelFormatR8Snorm;

        case TextureFormat::R16:            return MTLPixelFormatR16Unorm;
        case TextureFormat::R16Sgn:         return MTLPixelFormatR16Snorm;
        case TextureFormat::R16Float:       return MTLPixelFormatR16Float;

        case TextureFormat::R32UInt:        return MTLPixelFormatR32Uint;
        case TextureFormat::R32SInt:        return MTLPixelFormatR32Sint;
        case TextureFormat::R32Float:       return MTLPixelFormatR32Float;

        case TextureFormat::RG8:            return MTLPixelFormatRG8Unorm;
        case TextureFormat::RG8Sgn:         return MTLPixelFormatRG8Snorm;

        case TextureFormat::RG16:           return MTLPixelFormatRG16Unorm;
        case TextureFormat::RG16Sgn:        return MTLPixelFormatRG16Snorm;
        case TextureFormat::RG16Float:      return MTLPixelFormatRG16Float;

        case TextureFormat::RG32UInt:       return MTLPixelFormatRG32Uint;
        case TextureFormat::RG32SInt:       return MTLPixelFormatRG32Sint;
        case TextureFormat::RG32Float:      return MTLPixelFormatRG32Float;

        case TextureFormat::RGB8:           break;
        case TextureFormat::RGB8Sgn:        break;

        case TextureFormat::RGB16:          break;
        case TextureFormat::RGB16Sgn:       break;
        case TextureFormat::RGB16Float:     break;

        case TextureFormat::RGB32UInt:      break;
        case TextureFormat::RGB32SInt:      break;
        case TextureFormat::RGB32Float:     break;

        case TextureFormat::RGBA8:          return MTLPixelFormatRGBA8Unorm;
        case TextureFormat::RGBA8Sgn:       return MTLPixelFormatRGBA8Snorm;

        case TextureFormat::RGBA16:         return MTLPixelFormatRGBA16Unorm;
        case TextureFormat::RGBA16Sgn:      return MTLPixelFormatRGBA16Snorm;
        case TextureFormat::RGBA16Float:    return MTLPixelFormatRGBA16Float;

        case TextureFormat::RGBA32UInt:     return MTLPixelFormatRGBA32Uint;
        case TextureFormat::RGBA32SInt:     return MTLPixelFormatRGBA32Sint;
        case TextureFormat::RGBA32Float:    return MTLPixelFormatRGBA32Float;

        /* --- Depth-stencil formats --- */
        case TextureFormat::D32:            return MTLPixelFormatDepth32Float;
        case TextureFormat::D24S8:          return MTLPixelFormatDepth24Unorm_Stencil8;

        /* --- Compressed color formats --- */
        case TextureFormat::RGB_DXT1:       break;
        case TextureFormat::RGBA_DXT1:      return MTLPixelFormatBC1_RGBA;
        case TextureFormat::RGBA_DXT3:      return MTLPixelFormatBC2_RGBA;
        case TextureFormat::RGBA_DXT5:      return MTLPixelFormatBC3_RGBA;
    }
    MapFailed("TextureFormat", "MTLPixelFormat");
}

MTLTextureType Map(const TextureType textureType)
{
    switch (textureType)
    {
        case TextureType::Texture1D:        return MTLTextureType1D;
        case TextureType::Texture2D:        return MTLTextureType2D;
        case TextureType::Texture3D:        return MTLTextureType3D;
        case TextureType::TextureCube:      return MTLTextureTypeCube;
        case TextureType::Texture1DArray:   return MTLTextureType1DArray;
        case TextureType::Texture2DArray:   return MTLTextureType2DArray;
        case TextureType::TextureCubeArray: return MTLTextureTypeCubeArray;
        case TextureType::Texture2DMS:      return MTLTextureType2DMultisample;
        case TextureType::Texture2DMSArray: break;//return MTLTextureType2DMultisampleArray; // Beta
    }
    MapFailed("TextureType", "MTLTextureType");
}

MTLPrimitiveType Map(const PrimitiveTopology primitiveTopology)
{
    switch (primitiveTopology)
    {
        case PrimitiveTopology::PointList:      return MTLPrimitiveTypePoint;
        case PrimitiveTopology::LineList:       return MTLPrimitiveTypeLine;
        case PrimitiveTopology::LineStrip:      return MTLPrimitiveTypeLineStrip;
        case PrimitiveTopology::TriangleList:   return MTLPrimitiveTypeTriangle;
        case PrimitiveTopology::TriangleStrip:  return MTLPrimitiveTypeTriangleStrip;
        default:                                break;
    }
    MapFailed("PrimitiveTopology", "MTLPrimitiveType");
}

MTLCullMode Map(const CullMode cullMode)
{
    switch (cullMode)
    {
        case CullMode::Disabled:    return MTLCullModeNone;
        case CullMode::Front:       return MTLCullModeFront;
        case CullMode::Back:        return MTLCullModeBack;
    }
    MapFailed("CullMode", "MTLCullMode");
}


} // /namespace MTTypes

} // /namespace LLGL



// ================================================================================
