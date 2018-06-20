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

MTLPixelFormat Map(const Format format)
{
    switch (format)
    {
        case Format::Undefined:       	break;

        /* --- Color formats --- */
        case Format::R8UNorm:        	return MTLPixelFormatR8Unorm;
        case Format::R8SNorm:        	return MTLPixelFormatR8Snorm;
        case Format::R8UInt:         	return MTLPixelFormatR8Uint;
        case Format::R8SInt:         	return MTLPixelFormatR8Sint;

        case Format::R16UNorm:       	return MTLPixelFormatR16Unorm;
        case Format::R16SNorm:       	return MTLPixelFormatR16Snorm;
        case Format::R16UInt:       	return MTLPixelFormatR16Uint;
        case Format::R16SInt:       	return MTLPixelFormatR16Sint;
        case Format::R16Float:       	return MTLPixelFormatR16Float;

        case Format::R32UInt:        	return MTLPixelFormatR32Uint;
        case Format::R32SInt:           return MTLPixelFormatR32Sint;
        case Format::R32Float:          return MTLPixelFormatR32Float;

        case Format::RG8UNorm:       	return MTLPixelFormatRG8Unorm;
        case Format::RG8SNorm:       	return MTLPixelFormatRG8Snorm;
        case Format::RG8UInt:       	return MTLPixelFormatRG8Unorm;
        case Format::RG8SInt:           return MTLPixelFormatRG8Snorm;

        case Format::RG16UNorm:      	return MTLPixelFormatRG16Unorm;
        case Format::RG16SNorm:      	return MTLPixelFormatRG16Snorm;
        case Format::RG16UInt:          return MTLPixelFormatRG16Uint;
        case Format::RG16SInt:          return MTLPixelFormatRG16Sint;
        case Format::RG16Float:         return MTLPixelFormatRG16Float;

        case Format::RG32UInt:          return MTLPixelFormatRG32Uint;
        case Format::RG32SInt:          return MTLPixelFormatRG32Sint;
        case Format::RG32Float:         return MTLPixelFormatRG32Float;

        case Format::RGB8UNorm:      	break;
        case Format::RGB8SNorm:         break;
        case Format::RGB8UInt:          break;
        case Format::RGB8SInt:          break;

        case Format::RGB16UNorm:        break;
        case Format::RGB16SNorm:        break;
        case Format::RGB16UInt:         break;
        case Format::RGB16SInt:         break;
        case Format::RGB16Float:     	break;

        case Format::RGB32UInt:         break;
        case Format::RGB32SInt:         break;
        case Format::RGB32Float:        break;

        case Format::RGBA8UNorm:        return MTLPixelFormatRGBA8Unorm;
        case Format::RGBA8SNorm:        return MTLPixelFormatRGBA8Snorm;
        case Format::RGBA8UInt:         return MTLPixelFormatRGBA8Unorm;
        case Format::RGBA8SInt:         return MTLPixelFormatRGBA8Snorm;

        case Format::RGBA16UNorm:       return MTLPixelFormatRGBA16Unorm;
        case Format::RGBA16SNorm:       return MTLPixelFormatRGBA16Snorm;
        case Format::RGBA16UInt:        return MTLPixelFormatRGBA16Uint;
        case Format::RGBA16SInt:        return MTLPixelFormatRGBA16Sint;
        case Format::RGBA16Float:       return MTLPixelFormatRGBA16Float;

        case Format::RGBA32UInt:        return MTLPixelFormatRGBA32Uint;
        case Format::RGBA32SInt:        return MTLPixelFormatRGBA32Sint;
        case Format::RGBA32Float:       return MTLPixelFormatRGBA32Float;

        /* --- Extended color formats --- */
        case Format::R64Float:          break;
        case Format::RG64Float:         break;
        case Format::RGB64Float:        break;
        case Format::RGBA64Float:       break;

        /* --- Depth-stencil formats --- */
        case Format::D16UNorm:          return MTLPixelFormatDepth16Unorm;
        case Format::D32Float:          return MTLPixelFormatDepth32Float;
        case Format::D24UNormS8UInt:    return MTLPixelFormatDepth24Unorm_Stencil8;
        case Format::D32FloatS8X24UInt: return MTLPixelFormatDepth32Float_Stencil8;

        /* --- Compressed color formats --- */
        case Format::BC1RGB:            break;
        case Format::BC1RGBA:           return MTLPixelFormatBC1_RGBA;
        case Format::BC2RGBA:           return MTLPixelFormatBC2_RGBA;
        case Format::BC3RGBA:           return MTLPixelFormatBC3_RGBA;
    }
    MapFailed("Format", "MTLPixelFormat");
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

MTLCompareFunction Map(const CompareOp compareOp)
{
    switch (compareOp)
    {
        case CompareOp::Never:          return MTLCompareFunctionNever;
        case CompareOp::Less:           return MTLCompareFunctionLess;
        case CompareOp::Equal:          return MTLCompareFunctionEqual;
        case CompareOp::LessEqual:      return MTLCompareFunctionLessEqual;
        case CompareOp::Greater:        return MTLCompareFunctionGreater;
        case CompareOp::NotEqual:       return MTLCompareFunctionNotEqual;
        case CompareOp::GreaterEqual:   return MTLCompareFunctionGreaterEqual;
        case CompareOp::Ever:           return MTLCompareFunctionAlways;
    }
    MapFailed("CompareOp", "MTLCompareFunction");
}

MTLVertexFormat ToMTLVertexFormat(const Format format)
{
    switch (format)
    {
        /* --- Color formats --- */
        case Format::R8UNorm:       return MTLVertexFormatUCharNormalized;
        case Format::R8SNorm:       return MTLVertexFormatCharNormalized;
        case Format::R8UInt:           return MTLVertexFormatUChar;
        case Format::R8SInt:        return MTLVertexFormatChar;
        
        case Format::R16UNorm:      return MTLVertexFormatUShortNormalized;
        case Format::R16SNorm:      return MTLVertexFormatShortNormalized;
        case Format::R16UInt:       return MTLVertexFormatUShort;
        case Format::R16SInt:       return MTLVertexFormatShort;
        case Format::R16Float:      break;

        case Format::R32UInt:       return MTLVertexFormatUInt;
        case Format::R32SInt:       return MTLVertexFormatInt;
        case Format::R32Float:      return MTLVertexFormatFloat;

        case Format::RG8UNorm:      return MTLVertexFormatUChar2Normalized;
        case Format::RG8SNorm:      return MTLVertexFormatChar2Normalized;
        case Format::RG8UInt:       return MTLVertexFormatUChar2;
        case Format::RG8SInt:       return MTLVertexFormatChar2;

        case Format::RG16UNorm:     return MTLVertexFormatUShort2Normalized;
        case Format::RG16SNorm:     return MTLVertexFormatShort2Normalized;
        case Format::RG16UInt:      return MTLVertexFormatUShort2;
        case Format::RG16SInt:      return MTLVertexFormatShort2;
        case Format::RG16Float:     break;

        case Format::RG32UInt:      return MTLVertexFormatUInt2;
        case Format::RG32SInt:      return MTLVertexFormatInt2;
        case Format::RG32Float:     return MTLVertexFormatFloat2;

        case Format::RGB8UNorm:     return MTLVertexFormatUChar3Normalized;
        case Format::RGB8SNorm:     return MTLVertexFormatChar3Normalized;
        case Format::RGB8UInt:      return MTLVertexFormatUChar3;
        case Format::RGB8SInt:      return MTLVertexFormatChar3;

        case Format::RGB16UNorm:    return MTLVertexFormatUShort3Normalized;
        case Format::RGB16SNorm:    return MTLVertexFormatShort3Normalized;
        case Format::RGB16UInt:     return MTLVertexFormatUShort3;
        case Format::RGB16SInt:     return MTLVertexFormatShort3;
        case Format::RGB16Float:    break;

        case Format::RGB32UInt:     return MTLVertexFormatUInt3;
        case Format::RGB32SInt:     return MTLVertexFormatInt3;
        case Format::RGB32Float:    return MTLVertexFormatFloat3;

        case Format::RGBA8UNorm:    return MTLVertexFormatUChar4Normalized;
        case Format::RGBA8SNorm:    return MTLVertexFormatChar4Normalized;
        case Format::RGBA8UInt:     return MTLVertexFormatUChar4;
        case Format::RGBA8SInt:     return MTLVertexFormatChar4;

        case Format::RGBA16UNorm:   return MTLVertexFormatUShort4Normalized;
        case Format::RGBA16SNorm:   return MTLVertexFormatShort4Normalized;
        case Format::RGBA16UInt:    return MTLVertexFormatUShort4;
        case Format::RGBA16SInt:    return MTLVertexFormatShort4;
        case Format::RGBA16Float:   break;

        case Format::RGBA32UInt:    return MTLVertexFormatUInt4;
        case Format::RGBA32SInt:    return MTLVertexFormatInt4;
        case Format::RGBA32Float:   return MTLVertexFormatFloat4;
        
        default:                    break;
    }
    MapFailed("Format", "MTLVertexFormat");
}


} // /namespace MTTypes

} // /namespace LLGL



// ================================================================================
