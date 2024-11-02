/*
 * MTTypes.mm
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "MTTypes.h"
#include "../../Core/Exception.h"


namespace LLGL
{

namespace MTTypes
{


[[noreturn]]
void MapFailed(const char* typeName, const char* mtlTypeName)
{
    LLGL_TRAP("failed to map <LLGL::%s> to <%s> Metal parameter", typeName, mtlTypeName);
}

[[noreturn]]
void UnmapFailed(const char* typeName, const char* mtlTypeName)
{
    LLGL_TRAP("failed to unmap <LLGL::%s> from <%s> Metal parameter", typeName, mtlTypeName);
}

MTLDataType ToMTLDataType(const DataType dataType)
{
    switch (dataType)
    {
        case DataType::Undefined:   return MTLDataTypeNone;

        case DataType::Int8:        return MTLDataTypeChar;
        case DataType::UInt8:       return MTLDataTypeUChar;

        case DataType::Int16:       return MTLDataTypeShort;
        case DataType::UInt16:      return MTLDataTypeUShort;

        case DataType::Int32:       return MTLDataTypeInt;
        case DataType::UInt32:      return MTLDataTypeUInt;

        case DataType::Float16:     return MTLDataTypeHalf;
        case DataType::Float32:     return MTLDataTypeFloat;
        case DataType::Float64:     break;
    }
    MapFailed("DataType", "MTLDataType");
}

MTLPixelFormat ToMTLPixelFormat(const Format format)
{
    switch (format)
    {
        case Format::Undefined:       	return MTLPixelFormatInvalid;

        /* --- Alpha channel color formats --- */
        case Format::A8UNorm:        	return MTLPixelFormatA8Unorm;

        /* --- Red channel color formats --- */
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

        case Format::R64Float:          break;

        /* --- RG color formats --- */
        case Format::RG8UNorm:       	return MTLPixelFormatRG8Unorm;
        case Format::RG8SNorm:       	return MTLPixelFormatRG8Snorm;
        case Format::RG8UInt:       	return MTLPixelFormatRG8Uint;
        case Format::RG8SInt:           return MTLPixelFormatRG8Sint;

        case Format::RG16UNorm:      	return MTLPixelFormatRG16Unorm;
        case Format::RG16SNorm:      	return MTLPixelFormatRG16Snorm;
        case Format::RG16UInt:          return MTLPixelFormatRG16Uint;
        case Format::RG16SInt:          return MTLPixelFormatRG16Sint;
        case Format::RG16Float:         return MTLPixelFormatRG16Float;

        case Format::RG32UInt:          return MTLPixelFormatRG32Uint;
        case Format::RG32SInt:          return MTLPixelFormatRG32Sint;
        case Format::RG32Float:         return MTLPixelFormatRG32Float;

        case Format::RG64Float:         break;

        /* --- RGB color formats --- */
        case Format::RGB8UNorm:      	break;
        case Format::RGB8UNorm_sRGB:    break;
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

        case Format::RGB64Float:        break;

        /* --- RGBA color formats --- */
        case Format::RGBA8UNorm:        return MTLPixelFormatRGBA8Unorm;
        case Format::RGBA8UNorm_sRGB:   return MTLPixelFormatRGBA8Unorm_sRGB;
        case Format::RGBA8SNorm:        return MTLPixelFormatRGBA8Snorm;
        case Format::RGBA8UInt:         return MTLPixelFormatRGBA8Uint;
        case Format::RGBA8SInt:         return MTLPixelFormatRGBA8Sint;

        case Format::RGBA16UNorm:       return MTLPixelFormatRGBA16Unorm;
        case Format::RGBA16SNorm:       return MTLPixelFormatRGBA16Snorm;
        case Format::RGBA16UInt:        return MTLPixelFormatRGBA16Uint;
        case Format::RGBA16SInt:        return MTLPixelFormatRGBA16Sint;
        case Format::RGBA16Float:       return MTLPixelFormatRGBA16Float;

        case Format::RGBA32UInt:        return MTLPixelFormatRGBA32Uint;
        case Format::RGBA32SInt:        return MTLPixelFormatRGBA32Sint;
        case Format::RGBA32Float:       return MTLPixelFormatRGBA32Float;

        case Format::RGBA64Float:       break;

        /* --- BGRA color formats --- */
        case Format::BGRA8UNorm:        return MTLPixelFormatBGRA8Unorm;
        case Format::BGRA8UNorm_sRGB:   return MTLPixelFormatBGRA8Unorm_sRGB;
        case Format::BGRA8SNorm:        break;
        case Format::BGRA8UInt:         break;
        case Format::BGRA8SInt:         break;

        /* --- Packed formats --- */
        case Format::RGB10A2UNorm:      return MTLPixelFormatRGB10A2Unorm;
        case Format::RGB10A2UInt:       return MTLPixelFormatRGB10A2Uint;
        case Format::RG11B10Float:      return MTLPixelFormatRG11B10Float;
        case Format::RGB9E5Float:       return MTLPixelFormatRGB9E5Float;

        /* --- Depth-stencil formats --- */
        #ifdef LLGL_OS_IOS
        case Format::D16UNorm:          return MTLPixelFormatDepth32Float;
        #else
        case Format::D16UNorm:          return MTLPixelFormatDepth16Unorm;
        #endif
        case Format::D32Float:          return MTLPixelFormatDepth32Float;
        case Format::D24UNormS8UInt:    return MTLPixelFormatDepth32Float_Stencil8; // MTLPixelFormatDepth24Unorm_Stencil8 not supported?
        case Format::D32FloatS8X24UInt: return MTLPixelFormatDepth32Float_Stencil8;

        /* --- Block compression (BC) formats --- */
        #ifndef LLGL_OS_IOS
        case Format::BC1UNorm:          return MTLPixelFormatBC1_RGBA;
        case Format::BC1UNorm_sRGB:     return MTLPixelFormatBC1_RGBA_sRGB;
        case Format::BC2UNorm:          return MTLPixelFormatBC2_RGBA;
        case Format::BC2UNorm_sRGB:     return MTLPixelFormatBC2_RGBA_sRGB;
        case Format::BC3UNorm:          return MTLPixelFormatBC3_RGBA;
        case Format::BC3UNorm_sRGB:     return MTLPixelFormatBC3_RGBA_sRGB;
        case Format::BC4UNorm:          return MTLPixelFormatBC4_RUnorm;
        case Format::BC4SNorm:          return MTLPixelFormatBC4_RSnorm;
        case Format::BC5UNorm:          return MTLPixelFormatBC5_RGUnorm;
        case Format::BC5SNorm:          return MTLPixelFormatBC5_RGSnorm;
        #endif

        /* --- Advanced scalable texture compression (ASTC) formats --- */
        #ifdef LLGL_OS_IOS //TODO: available in macOS 11
        case Format::ASTC4x4:           return MTLPixelFormatASTC_4x4_LDR;
        case Format::ASTC4x4_sRGB:      return MTLPixelFormatASTC_4x4_sRGB;
        case Format::ASTC5x4:           return MTLPixelFormatASTC_5x4_LDR;
        case Format::ASTC5x4_sRGB:      return MTLPixelFormatASTC_5x4_sRGB;
        case Format::ASTC5x5:           return MTLPixelFormatASTC_5x5_LDR;
        case Format::ASTC5x5_sRGB:      return MTLPixelFormatASTC_5x5_sRGB;
        case Format::ASTC6x5:           return MTLPixelFormatASTC_6x5_LDR;
        case Format::ASTC6x5_sRGB:      return MTLPixelFormatASTC_6x5_sRGB;
        case Format::ASTC6x6:           return MTLPixelFormatASTC_6x6_LDR;
        case Format::ASTC6x6_sRGB:      return MTLPixelFormatASTC_6x6_sRGB;
        case Format::ASTC8x5:           return MTLPixelFormatASTC_8x5_LDR;
        case Format::ASTC8x5_sRGB:      return MTLPixelFormatASTC_8x5_sRGB;
        case Format::ASTC8x6:           return MTLPixelFormatASTC_8x6_LDR;
        case Format::ASTC8x6_sRGB:      return MTLPixelFormatASTC_8x6_sRGB;
        case Format::ASTC8x8:           return MTLPixelFormatASTC_8x8_LDR;
        case Format::ASTC8x8_sRGB:      return MTLPixelFormatASTC_8x8_sRGB;
        case Format::ASTC10x5:          return MTLPixelFormatASTC_10x5_LDR;
        case Format::ASTC10x5_sRGB:     return MTLPixelFormatASTC_10x5_sRGB;
        case Format::ASTC10x6:          return MTLPixelFormatASTC_10x6_LDR;
        case Format::ASTC10x6_sRGB:     return MTLPixelFormatASTC_10x6_sRGB;
        case Format::ASTC10x8:          return MTLPixelFormatASTC_10x8_LDR;
        case Format::ASTC10x8_sRGB:     return MTLPixelFormatASTC_10x8_sRGB;
        case Format::ASTC10x10:         return MTLPixelFormatASTC_10x10_LDR;
        case Format::ASTC10x10_sRGB:    return MTLPixelFormatASTC_10x10_sRGB;
        case Format::ASTC12x10:         return MTLPixelFormatASTC_12x10_LDR;
        case Format::ASTC12x10_sRGB:    return MTLPixelFormatASTC_12x10_sRGB;
        case Format::ASTC12x12:         return MTLPixelFormatASTC_12x12_LDR;
        case Format::ASTC12x12_sRGB:    return MTLPixelFormatASTC_12x12_sRGB;

        /* --- Ericsson texture compression (ETC) formats --- */
        case Format::ETC1UNorm:         break;
        case Format::ETC2UNorm:         return MTLPixelFormatETC2_RGB8;
        case Format::ETC2UNorm_sRGB:    return MTLPixelFormatETC2_RGB8_sRGB;
        #endif

        default:                        break;
    }
    MapFailed("Format", "MTLPixelFormat");
}

MTLVertexFormat ToMTLVertexFormat(const Format format)
{
    switch (format)
    {
        /* --- Red channel color formats --- */
        case Format::R8UNorm:       return MTLVertexFormatUCharNormalized;
        case Format::R8SNorm:       return MTLVertexFormatCharNormalized;
        case Format::R8UInt:        return MTLVertexFormatUChar;
        case Format::R8SInt:        return MTLVertexFormatChar;

        case Format::R16UNorm:      return MTLVertexFormatUShortNormalized;
        case Format::R16SNorm:      return MTLVertexFormatShortNormalized;
        case Format::R16UInt:       return MTLVertexFormatUShort;
        case Format::R16SInt:       return MTLVertexFormatShort;
        case Format::R16Float:      break;

        case Format::R32UInt:       return MTLVertexFormatUInt;
        case Format::R32SInt:       return MTLVertexFormatInt;
        case Format::R32Float:      return MTLVertexFormatFloat;

        /* --- RG color formats --- */
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

        /* --- RGB color formats --- */
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

        /* --- RGBA color formats --- */
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

MTLTessellationControlPointIndexType ToMTLPatchIndexType(const Format format)
{
    switch (format)
    {
        case Format::Undefined: return MTLTessellationControlPointIndexTypeNone;
        case Format::R16UInt:   return MTLTessellationControlPointIndexTypeUInt16;
        case Format::R32UInt:   return MTLTessellationControlPointIndexTypeUInt32;
        default:                break;
    }
    MapFailed("Format", "MTLTessellationControlPointIndexType");
}

MTLTessellationPartitionMode ToMTLPartitionMode(const TessellationPartition partitionMode)
{
    switch (partitionMode)
    {
        case TessellationPartition::Undefined:      break;
        case TessellationPartition::Integer:        return MTLTessellationPartitionModeInteger;
        case TessellationPartition::Pow2:           return MTLTessellationPartitionModePow2;
        case TessellationPartition::FractionalOdd:  return MTLTessellationPartitionModeFractionalOdd;
        case TessellationPartition::FractionalEven: return MTLTessellationPartitionModeFractionalEven;
    }
    MapFailed("TessellationPartition", "MTLTessellationPartitionMode");
}

MTLTextureType ToMTLTextureType(const TextureType textureType)
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
        case TextureType::Texture2DMSArray:
            #ifndef LLGL_OS_IOS
            if (@available(macOS 10.14, *))
                return MTLTextureType2DMultisampleArray;
            #endif
            break;
    }
    MapFailed("TextureType", "MTLTextureType");
}

MTLPrimitiveType ToMTLPrimitiveType(const PrimitiveTopology primitiveTopology)
{
    switch (primitiveTopology)
    {
        case PrimitiveTopology::PointList:      return MTLPrimitiveTypePoint;
        case PrimitiveTopology::LineList:       return MTLPrimitiveTypeLine;
        case PrimitiveTopology::LineStrip:      return MTLPrimitiveTypeLineStrip;
        case PrimitiveTopology::TriangleList:   return MTLPrimitiveTypeTriangle;
        case PrimitiveTopology::TriangleStrip:  return MTLPrimitiveTypeTriangleStrip;
        default:
            if (IsPrimitiveTopologyPatches(primitiveTopology))
                return MTLPrimitiveTypePoint;
            break;
    }
    MapFailed("PrimitiveTopology", "MTLPrimitiveType");
}

MTLPrimitiveTopologyClass ToMTLPrimitiveTopologyClass(const PrimitiveTopology primitiveTopology)
{
    switch (primitiveTopology)
    {
        case PrimitiveTopology::PointList:      return MTLPrimitiveTopologyClassPoint;
        case PrimitiveTopology::LineList:       /* pass */
        case PrimitiveTopology::LineStrip:      return MTLPrimitiveTopologyClassLine;
        case PrimitiveTopology::TriangleList:   /* pass */
        case PrimitiveTopology::TriangleStrip:  return MTLPrimitiveTopologyClassTriangle;
        default:
            if (IsPrimitiveTopologyPatches(primitiveTopology))
                return MTLPrimitiveTopologyClassUnspecified;
            break;
    }
    MapFailed("PrimitiveTopology", "MTLPrimitiveTopologyClass");
}

MTLCullMode ToMTLCullMode(const CullMode cullMode)
{
    switch (cullMode)
    {
        case CullMode::Disabled:    return MTLCullModeNone;
        case CullMode::Front:       return MTLCullModeFront;
        case CullMode::Back:        return MTLCullModeBack;
    }
    MapFailed("CullMode", "MTLCullMode");
}

MTLCompareFunction ToMTLCompareFunction(const CompareOp compareOp)
{
    switch (compareOp)
    {
        case CompareOp::NeverPass:      return MTLCompareFunctionNever;
        case CompareOp::Less:           return MTLCompareFunctionLess;
        case CompareOp::Equal:          return MTLCompareFunctionEqual;
        case CompareOp::LessEqual:      return MTLCompareFunctionLessEqual;
        case CompareOp::Greater:        return MTLCompareFunctionGreater;
        case CompareOp::NotEqual:       return MTLCompareFunctionNotEqual;
        case CompareOp::GreaterEqual:   return MTLCompareFunctionGreaterEqual;
        case CompareOp::AlwaysPass:     return MTLCompareFunctionAlways;
    }
    MapFailed("CompareOp", "MTLCompareFunction");
}

MTLSamplerAddressMode ToMTLSamplerAddressMode(const SamplerAddressMode addressMode)
{
    switch (addressMode)
    {
        case SamplerAddressMode::Repeat:        return MTLSamplerAddressModeRepeat;
        case SamplerAddressMode::Mirror:        return MTLSamplerAddressModeMirrorRepeat;
        case SamplerAddressMode::Clamp:         return MTLSamplerAddressModeClampToEdge;
        #ifndef LLGL_OS_IOS
        case SamplerAddressMode::Border:        return MTLSamplerAddressModeClampToBorderColor;
        case SamplerAddressMode::MirrorOnce:    return MTLSamplerAddressModeMirrorClampToEdge;
        #else
        case SamplerAddressMode::Border:        return MTLSamplerAddressModeClampToZero; // Workaround for iOS
        case SamplerAddressMode::MirrorOnce:    break;
        #endif // /LLGL_OS_IOS
    }
    MapFailed("SamplerAddressMode", "MTLSamplerAddressMode");
}

MTLSamplerMinMagFilter ToMTLSamplerMinMagFilter(const SamplerFilter filter)
{
    switch (filter)
    {
        case SamplerFilter::Nearest:    return MTLSamplerMinMagFilterNearest;
        case SamplerFilter::Linear:     return MTLSamplerMinMagFilterLinear;
    }
    MapFailed("SamplerFilter", "MTLSamplerMinMagFilter");
}

MTLSamplerMipFilter ToMTLSamplerMipFilter(const SamplerFilter filter)
{
    switch (filter)
    {
        case SamplerFilter::Nearest:    return MTLSamplerMipFilterNearest;
        case SamplerFilter::Linear:     return MTLSamplerMipFilterLinear;
    }
    MapFailed("SamplerFilter", "MTLSamplerMipFilter");
}

MTLTriangleFillMode ToMTLTriangleFillMode (const PolygonMode polygonMode)
{
    switch (polygonMode)
    {
        case PolygonMode::Fill:         return MTLTriangleFillModeFill;
        case PolygonMode::Wireframe:    return MTLTriangleFillModeLines;
        case PolygonMode::Points:       break;
    }
    MapFailed("PolygonMode", "MTLTriangleFillMode");
}

MTLStencilOperation ToMTLStencilOperation(const StencilOp stencilOp)
{
    switch (stencilOp)
    {
        case StencilOp::Keep:       return MTLStencilOperationKeep;
        case StencilOp::Zero:       return MTLStencilOperationZero;
        case StencilOp::Replace:    return MTLStencilOperationReplace;
        case StencilOp::IncClamp:   return MTLStencilOperationIncrementClamp;
        case StencilOp::DecClamp:   return MTLStencilOperationDecrementClamp;
        case StencilOp::Invert:     return MTLStencilOperationInvert;
        case StencilOp::IncWrap:    return MTLStencilOperationIncrementWrap;
        case StencilOp::DecWrap:    return MTLStencilOperationDecrementWrap;
    }
    MapFailed("StencilOp", "MTLStencilOperation");
}

MTLLoadAction ToMTLLoadAction(const AttachmentLoadOp loadOp)
{
    switch (loadOp)
    {
        case AttachmentLoadOp::Undefined:   return MTLLoadActionDontCare;
        case AttachmentLoadOp::Load:        return MTLLoadActionLoad;
        case AttachmentLoadOp::Clear:       return MTLLoadActionClear;
    }
    MapFailed("AttachmentLoadOp", "MTLLoadAction");
}

MTLStoreAction ToMTLStoreAction(const AttachmentStoreOp storeOp)
{
    switch (storeOp)
    {
        case AttachmentStoreOp::Undefined:  return MTLStoreActionDontCare;
        case AttachmentStoreOp::Store:      return MTLStoreActionStore;
    }
    MapFailed("AttachmentStoreOp", "MTLStoreAction");
}

MTLBlendOperation ToMTLBlendOperation(const BlendArithmetic blendArithmetic)
{
    switch (blendArithmetic)
    {
        case BlendArithmetic::Add:          return MTLBlendOperationAdd;
        case BlendArithmetic::Subtract:     return MTLBlendOperationSubtract;
        case BlendArithmetic::RevSubtract:  return MTLBlendOperationReverseSubtract;
        case BlendArithmetic::Min:          return MTLBlendOperationMin;
        case BlendArithmetic::Max:          return MTLBlendOperationMax;
    }
    MapFailed("BlendArithmetic", "MTLBlendOperation");
}

MTLBlendFactor ToMTLBlendFactor(const BlendOp blendOp)
{
    switch (blendOp)
    {
        case BlendOp::Zero:             return MTLBlendFactorZero;
        case BlendOp::One:              return MTLBlendFactorOne;
        case BlendOp::SrcColor:         return MTLBlendFactorSourceColor;
        case BlendOp::InvSrcColor:      return MTLBlendFactorOneMinusSourceColor;
        case BlendOp::SrcAlpha:         return MTLBlendFactorSourceAlpha;
        case BlendOp::InvSrcAlpha:      return MTLBlendFactorOneMinusSourceAlpha;
        case BlendOp::DstColor:         return MTLBlendFactorDestinationColor;
        case BlendOp::InvDstColor:      return MTLBlendFactorOneMinusDestinationColor;
        case BlendOp::DstAlpha:         return MTLBlendFactorDestinationAlpha;
        case BlendOp::InvDstAlpha:      return MTLBlendFactorOneMinusDestinationAlpha;
        case BlendOp::SrcAlphaSaturate: return MTLBlendFactorSourceAlphaSaturated;
        case BlendOp::BlendFactor:      return MTLBlendFactorBlendColor;
        case BlendOp::InvBlendFactor:   return MTLBlendFactorOneMinusBlendColor;
        case BlendOp::Src1Color:        return MTLBlendFactorSource1Color;
        case BlendOp::InvSrc1Color:     return MTLBlendFactorOneMinusSource1Color;
        case BlendOp::Src1Alpha:        return MTLBlendFactorSource1Alpha;
        case BlendOp::InvSrc1Alpha:     return MTLBlendFactorOneMinusSource1Alpha;
    }
    MapFailed("BlendOp", "MTLBlendFactor");
}

MTLClearColor ToMTLClearColor(const float color[4])
{
    return MTLClearColorMake(
        static_cast<double>(color[0]),
        static_cast<double>(color[1]),
        static_cast<double>(color[2]),
        static_cast<double>(color[3])
    );
}

API_AVAILABLE(macos(10.15), ios(13.0))
MTLTextureSwizzle ToMTLTextureSwizzle(const TextureSwizzle swizzle)
{
    switch (swizzle)
    {
        case TextureSwizzle::Zero:  return MTLTextureSwizzleZero;
        case TextureSwizzle::One:   return MTLTextureSwizzleOne;
        case TextureSwizzle::Red:   return MTLTextureSwizzleRed;
        case TextureSwizzle::Green: return MTLTextureSwizzleGreen;
        case TextureSwizzle::Blue:  return MTLTextureSwizzleBlue;
        case TextureSwizzle::Alpha: return MTLTextureSwizzleAlpha;
    }
    MapFailed("TextureSwizzle", "MTLTextureSwizzle");
}

Format ToFormat(const MTLPixelFormat pixelFormat)
{
    switch (pixelFormat)
    {
        case MTLPixelFormatInvalid:                 return Format::Undefined;

        /* --- Alpha channel color formats --- */
        case MTLPixelFormatA8Unorm:                 return Format::A8UNorm;

        /* --- Red channel color formats --- */
        case MTLPixelFormatR8Unorm:            		return Format::R8UNorm;
        case MTLPixelFormatR8Snorm:            		return Format::R8SNorm;
        case MTLPixelFormatR8Uint:             		return Format::R8UInt;
        case MTLPixelFormatR8Sint:             		return Format::R8SInt;

        case MTLPixelFormatR16Unorm:           		return Format::R16UNorm;
        case MTLPixelFormatR16Snorm:           		return Format::R16SNorm;
        case MTLPixelFormatR16Uint:                 return Format::R16UInt;
        case MTLPixelFormatR16Sint:           		return Format::R16SInt;
        case MTLPixelFormatR16Float:           		return Format::R16Float;

        case MTLPixelFormatR32Uint:            		return Format::R32UInt;
        case MTLPixelFormatR32Sint:           		return Format::R32SInt;
        case MTLPixelFormatR32Float:          		return Format::R32Float;

        /* --- RG color formats --- */
        case MTLPixelFormatRG8Unorm:           		return Format::RG8UNorm;
        case MTLPixelFormatRG8Snorm:           		return Format::RG8SNorm;
        case MTLPixelFormatRG8Uint:           		return Format::RG8UInt;
        case MTLPixelFormatRG8Sint:           		return Format::RG8SInt;

        case MTLPixelFormatRG16Unorm:          		return Format::RG16UNorm;
        case MTLPixelFormatRG16Snorm:          		return Format::RG16SNorm;
        case MTLPixelFormatRG16Uint:          		return Format::RG16UInt;
        case MTLPixelFormatRG16Sint:          		return Format::RG16SInt;
        case MTLPixelFormatRG16Float:         		return Format::RG16Float;

        case MTLPixelFormatRG32Uint:          		return Format::RG32UInt;
        case MTLPixelFormatRG32Sint:          		return Format::RG32SInt;
        case MTLPixelFormatRG32Float:         		return Format::RG32Float;

        /* --- RGBA color formats --- */
        case MTLPixelFormatRGBA8Unorm:        		return Format::RGBA8UNorm;
        case MTLPixelFormatRGBA8Snorm:        		return Format::RGBA8SNorm;
        case MTLPixelFormatRGBA8Uint:         		return Format::RGBA8UInt;
        case MTLPixelFormatRGBA8Sint:         		return Format::RGBA8SInt;

        case MTLPixelFormatRGBA16Unorm:             return Format::RGBA16UNorm;
        case MTLPixelFormatRGBA16Snorm:       	    return Format::RGBA16SNorm;
        case MTLPixelFormatRGBA16Uint:              return Format::RGBA16UInt;
        case MTLPixelFormatRGBA16Sint:              return Format::RGBA16SInt;
        case MTLPixelFormatRGBA16Float:             return Format::RGBA16Float;

        case MTLPixelFormatRGBA32Uint:              return Format::RGBA32UInt;
        case MTLPixelFormatRGBA32Sint:              return Format::RGBA32SInt;
        case MTLPixelFormatRGBA32Float:             return Format::RGBA32Float;

        /* --- BGRA color formats --- */
        case MTLPixelFormatBGRA8Unorm:              return Format::BGRA8UNorm;
        case MTLPixelFormatBGRA8Unorm_sRGB:         return Format::BGRA8UNorm_sRGB;

        /* --- Packed formats --- */
        case MTLPixelFormatRGB10A2Unorm:            return Format::RGB10A2UNorm;
        case MTLPixelFormatRGB10A2Uint:             return Format::RGB10A2UInt;
        case MTLPixelFormatRG11B10Float:            return Format::RG11B10Float;
        case MTLPixelFormatRGB9E5Float:             return Format::RGB9E5Float;

        /* --- Depth-stencil formats --- */
        #ifndef LLGL_OS_IOS
        case MTLPixelFormatDepth16Unorm:            return Format::D16UNorm;
        #endif // /LLGL_OS_IOS
        case MTLPixelFormatDepth32Float:            return Format::D32Float;
        #ifndef LLGL_OS_IOS
        case MTLPixelFormatDepth24Unorm_Stencil8:   return Format::D24UNormS8UInt;
        #endif // /LLGL_OS_IOS
        case MTLPixelFormatDepth32Float_Stencil8:   return Format::D32FloatS8X24UInt;

        /* --- Compressed color formats --- */
        #ifndef LLGL_OS_IOS
        case MTLPixelFormatBC1_RGBA:                return Format::BC1UNorm;
        case MTLPixelFormatBC1_RGBA_sRGB:           return Format::BC1UNorm_sRGB;
        case MTLPixelFormatBC2_RGBA:                return Format::BC2UNorm;
        case MTLPixelFormatBC2_RGBA_sRGB:           return Format::BC2UNorm_sRGB;
        case MTLPixelFormatBC3_RGBA:                return Format::BC3UNorm;
        case MTLPixelFormatBC3_RGBA_sRGB:           return Format::BC3UNorm_sRGB;
        case MTLPixelFormatBC4_RUnorm:              return Format::BC4UNorm;
        case MTLPixelFormatBC4_RSnorm:              return Format::BC4SNorm;
        case MTLPixelFormatBC5_RGUnorm:             return Format::BC5UNorm;
        case MTLPixelFormatBC5_RGSnorm:             return Format::BC5SNorm;
        #endif // /LLGL_OS_IOS

        /* --- Advanced scalable texture compression (ASTC) formats --- */
        #ifdef LLGL_OS_IOS //TODO: available in macOS 11
        case MTLPixelFormatASTC_4x4_LDR:            return Format::ASTC4x4;
        case MTLPixelFormatASTC_4x4_sRGB:           return Format::ASTC4x4_sRGB;
        case MTLPixelFormatASTC_5x4_LDR:            return Format::ASTC5x4;
        case MTLPixelFormatASTC_5x4_sRGB:           return Format::ASTC5x4_sRGB;
        case MTLPixelFormatASTC_5x5_LDR:            return Format::ASTC5x5;
        case MTLPixelFormatASTC_5x5_sRGB:           return Format::ASTC5x5_sRGB;
        case MTLPixelFormatASTC_6x5_LDR:            return Format::ASTC6x5;
        case MTLPixelFormatASTC_6x5_sRGB:           return Format::ASTC6x5_sRGB;
        case MTLPixelFormatASTC_6x6_LDR:            return Format::ASTC6x6;
        case MTLPixelFormatASTC_6x6_sRGB:           return Format::ASTC6x6_sRGB;
        case MTLPixelFormatASTC_8x5_LDR:            return Format::ASTC8x5;
        case MTLPixelFormatASTC_8x5_sRGB:           return Format::ASTC8x5_sRGB;
        case MTLPixelFormatASTC_8x6_LDR:            return Format::ASTC8x6;
        case MTLPixelFormatASTC_8x6_sRGB:           return Format::ASTC8x6_sRGB;
        case MTLPixelFormatASTC_8x8_LDR:            return Format::ASTC8x8;
        case MTLPixelFormatASTC_8x8_sRGB:           return Format::ASTC8x8_sRGB;
        case MTLPixelFormatASTC_10x5_LDR:           return Format::ASTC10x5;
        case MTLPixelFormatASTC_10x5_sRGB:          return Format::ASTC10x5_sRGB;
        case MTLPixelFormatASTC_10x6_LDR:           return Format::ASTC10x6;
        case MTLPixelFormatASTC_10x6_sRGB:          return Format::ASTC10x6_sRGB;
        case MTLPixelFormatASTC_10x8_LDR:           return Format::ASTC10x8;
        case MTLPixelFormatASTC_10x8_sRGB:          return Format::ASTC10x8_sRGB;
        case MTLPixelFormatASTC_10x10_LDR:          return Format::ASTC10x10;
        case MTLPixelFormatASTC_10x10_sRGB:         return Format::ASTC10x10_sRGB;
        case MTLPixelFormatASTC_12x10_LDR:          return Format::ASTC12x10;
        case MTLPixelFormatASTC_12x10_sRGB:         return Format::ASTC12x10_sRGB;
        case MTLPixelFormatASTC_12x12_LDR:          return Format::ASTC12x12;
        case MTLPixelFormatASTC_12x12_sRGB:         return Format::ASTC12x12_sRGB;

        /* --- Ericsson texture compression (ETC) formats --- */
        case MTLPixelFormatETC2_RGB8:               return Format::ETC2UNorm;
        case MTLPixelFormatETC2_RGB8_sRGB:          return Format::ETC2UNorm_sRGB;
        #endif

        default:                                    break;
    }
    UnmapFailed("Format", "MTLPixelFormat");
}

void Convert(MTLOrigin& dst, const Offset3D& src)
{
    dst.x = static_cast<NSUInteger>(std::max(0, src.x));
    dst.y = static_cast<NSUInteger>(std::max(0, src.y));
    dst.z = static_cast<NSUInteger>(std::max(0, src.z));
}

void Convert(MTLSize& dst, const Extent3D& src)
{
    dst.width   = static_cast<NSUInteger>(src.width );
    dst.height  = static_cast<NSUInteger>(src.height);
    dst.depth   = static_cast<NSUInteger>(src.depth );
}

API_AVAILABLE(macos(10.15), ios(13.0))
void Convert(MTLTextureSwizzleChannels& dst, const TextureSwizzleRGBA& src)
{
    dst.red     = ToMTLTextureSwizzle(src.r);
    dst.green   = ToMTLTextureSwizzle(src.g);
    dst.blue    = ToMTLTextureSwizzle(src.b);
    dst.alpha   = ToMTLTextureSwizzle(src.a);
}


} // /namespace MTTypes

} // /namespace LLGL



// ================================================================================
