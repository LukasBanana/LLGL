/*
 * WGTypes.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "WGTypes.h"
#include "../../Core/Assertion.h"
#include <stdexcept>
#include <string>


namespace LLGL
{

namespace WGTypes
{


[[noreturn]]
void MapFailed(const char* typeName, const char* wgpuTypeName)
{
    LLGL_TRAP("failed to map <LLGL::%s> to <%s> WebGPU parameter", typeName, wgpuTypeName);
}

WGPUIndexFormat ToWGIndexFormat(const Format format)
{
    switch (format)
    {
        case Format::Undefined: return WGPUIndexFormat_Undefined;
        case Format::R16UInt:   return WGPUIndexFormat_Uint16;
        case Format::R32UInt:   return WGPUIndexFormat_Uint32;
        default:                break;
    }
    MapFailed("Format", "WGPUIndexFormat");
}

WGPUTextureFormat ToWGTextureFormat(const Format format)
{
    WGPUTextureFormat outFormat = ToWGTextureFormatOrDefault(format);
    if (outFormat != WGPUTextureFormat_Undefined)
        return outFormat;
    MapFailed("Format", "WGPUTextureFormat");
}

WGPUTextureFormat ToWGTextureFormatOrDefault(const Format format)
{
    switch (format)
    {
        /* --- Alpha channel color formats --- */
        case Format::A8UNorm:               return WGPUTextureFormat_R8Unorm; // Swizzling

        /* --- Red channel color formats --- */
        case Format::R8UNorm:               return WGPUTextureFormat_R8Unorm;
        case Format::R8SNorm:               return WGPUTextureFormat_R8Snorm;
        case Format::R8UInt:                return WGPUTextureFormat_R8Uint;
        case Format::R8SInt:                return WGPUTextureFormat_R8Sint;

        case Format::R16UNorm:              return WGPUTextureFormat_R16Unorm;
        case Format::R16SNorm:              return WGPUTextureFormat_R16Snorm;
        case Format::R16UInt:               return WGPUTextureFormat_R16Uint;
        case Format::R16SInt:               return WGPUTextureFormat_R16Sint;
        case Format::R16Float:              return WGPUTextureFormat_R16Float;

        case Format::R32UInt:               return WGPUTextureFormat_R32Uint;
        case Format::R32SInt:               return WGPUTextureFormat_R32Sint;
        case Format::R32Float:              return WGPUTextureFormat_R32Float;

        case Format::R64Float:              break;

        /* --- RG color formats --- */
        case Format::RG8UNorm:              return WGPUTextureFormat_RG8Unorm;
        case Format::RG8SNorm:              return WGPUTextureFormat_RG8Snorm;
        case Format::RG8UInt:               return WGPUTextureFormat_RG8Uint;
        case Format::RG8SInt:               return WGPUTextureFormat_RG8Sint;

        case Format::RG16UNorm:             return WGPUTextureFormat_RG16Unorm;
        case Format::RG16SNorm:             return WGPUTextureFormat_RG16Snorm;
        case Format::RG16UInt:              return WGPUTextureFormat_RG16Uint;
        case Format::RG16SInt:              return WGPUTextureFormat_RG16Sint;
        case Format::RG16Float:             return WGPUTextureFormat_RG16Float;

        case Format::RG32UInt:              return WGPUTextureFormat_RG32Uint;
        case Format::RG32SInt:              return WGPUTextureFormat_RG32Sint;
        case Format::RG32Float:             return WGPUTextureFormat_RG32Float;

        case Format::RG64Float:             break;

        /* --- RGB color formats --- */
        case Format::RGB8UNorm:             break;
        case Format::RGB8UNorm_sRGB:        break;
        case Format::RGB8SNorm:             break;
        case Format::RGB8UInt:              break;
        case Format::RGB8SInt:              break;

        case Format::RGB16UNorm:            break;
        case Format::RGB16SNorm:            break;
        case Format::RGB16UInt:             break;
        case Format::RGB16SInt:             break;
        case Format::RGB16Float:            break;

        case Format::RGB32UInt:             break;
        case Format::RGB32SInt:             break;
        case Format::RGB32Float:            break;

        case Format::RGB64Float:            break;

        /* --- RGBA color formats --- */
        case Format::RGBA8UNorm:            return WGPUTextureFormat_RGBA8Unorm;
        case Format::RGBA8UNorm_sRGB:       return WGPUTextureFormat_RGBA8UnormSrgb;
        case Format::RGBA8SNorm:            return WGPUTextureFormat_RGBA8Snorm;
        case Format::RGBA8UInt:             return WGPUTextureFormat_RGBA8Uint;
        case Format::RGBA8SInt:             return WGPUTextureFormat_RGBA8Sint;

        case Format::RGBA16UNorm:           return WGPUTextureFormat_RGBA16Unorm;
        case Format::RGBA16SNorm:           return WGPUTextureFormat_RGBA16Snorm;
        case Format::RGBA16UInt:            return WGPUTextureFormat_RGBA16Uint;
        case Format::RGBA16SInt:            return WGPUTextureFormat_RGBA16Sint;
        case Format::RGBA16Float:           return WGPUTextureFormat_RGBA16Float;

        case Format::RGBA32UInt:            return WGPUTextureFormat_RGBA32Uint;
        case Format::RGBA32SInt:            return WGPUTextureFormat_RGBA32Sint;
        case Format::RGBA32Float:           return WGPUTextureFormat_RGBA32Float;

        case Format::RGBA64Float:           break;

        /* --- BGRA color formats --- */
        case Format::BGRA8UNorm:            return WGPUTextureFormat_BGRA8Unorm;
        case Format::BGRA8UNorm_sRGB:       return WGPUTextureFormat_BGRA8UnormSrgb;
        case Format::BGRA8SNorm:            break;
        case Format::BGRA8UInt:             break;
        case Format::BGRA8SInt:             break;

        /* --- Packed formats --- */
        case Format::RGB10A2UNorm:          return WGPUTextureFormat_RGB10A2Unorm;
        case Format::RGB10A2UInt:           return WGPUTextureFormat_RGB10A2Uint;
        case Format::RG11B10Float:          return WGPUTextureFormat_RG11B10Ufloat;
        case Format::RGB9E5Float:           return WGPUTextureFormat_RGB9E5Ufloat;
        case Format::BGR5A1UNorm:           break;

        /* --- Depth-stencil formats --- */
        case Format::D16UNorm:              return WGPUTextureFormat_Depth16Unorm;
        case Format::D24UNormS8UInt:        return WGPUTextureFormat_Depth24PlusStencil8;
        case Format::D32Float:              return WGPUTextureFormat_Depth32Float;
        case Format::D32FloatS8X24UInt:     return WGPUTextureFormat_Depth32FloatStencil8;
        // unsupported                      return WGPUTextureFormat_Stencil8;

        /* --- Block compression (BC) formats --- */
        case Format::BC1UNorm:              return WGPUTextureFormat_BC1RGBAUnorm;
        case Format::BC1UNorm_sRGB:         return WGPUTextureFormat_BC1RGBAUnormSrgb;
        case Format::BC2UNorm:              return WGPUTextureFormat_BC2RGBAUnorm;
        case Format::BC2UNorm_sRGB:         return WGPUTextureFormat_BC2RGBAUnormSrgb;
        case Format::BC3UNorm:              return WGPUTextureFormat_BC3RGBAUnorm;
        case Format::BC3UNorm_sRGB:         return WGPUTextureFormat_BC3RGBAUnormSrgb;
        case Format::BC4UNorm:              return WGPUTextureFormat_BC4RUnorm;
        case Format::BC4SNorm:              return WGPUTextureFormat_BC4RSnorm;
        case Format::BC5UNorm:              return WGPUTextureFormat_BC5RGUnorm;
        case Format::BC5SNorm:              return WGPUTextureFormat_BC5RGSnorm;
        case Format::BC6HUFloat:            return WGPUTextureFormat_BC6HRGBUfloat;
        case Format::BC6HSFloat:            return WGPUTextureFormat_BC6HRGBFloat;
        case Format::BC7UNorm:              return WGPUTextureFormat_BC7RGBAUnorm;
        case Format::BC7UNorm_sRGB:         return WGPUTextureFormat_BC7RGBAUnormSrgb;

        /* --- ASTC formats --- */
        case Format::ASTC4x4:               return WGPUTextureFormat_ASTC4x4Unorm;
        case Format::ASTC4x4_sRGB:          return WGPUTextureFormat_ASTC4x4UnormSrgb;
        case Format::ASTC5x4:               return WGPUTextureFormat_ASTC5x4Unorm;
        case Format::ASTC5x4_sRGB:          return WGPUTextureFormat_ASTC5x4UnormSrgb;
        case Format::ASTC5x5:               return WGPUTextureFormat_ASTC5x5Unorm;
        case Format::ASTC5x5_sRGB:          return WGPUTextureFormat_ASTC5x5UnormSrgb;
        case Format::ASTC6x5:               return WGPUTextureFormat_ASTC6x5Unorm;
        case Format::ASTC6x5_sRGB:          return WGPUTextureFormat_ASTC6x5UnormSrgb;
        case Format::ASTC6x6:               return WGPUTextureFormat_ASTC6x6Unorm;
        case Format::ASTC6x6_sRGB:          return WGPUTextureFormat_ASTC6x6UnormSrgb;
        case Format::ASTC8x5:               return WGPUTextureFormat_ASTC8x5Unorm;
        case Format::ASTC8x5_sRGB:          return WGPUTextureFormat_ASTC8x5UnormSrgb;
        case Format::ASTC8x6:               return WGPUTextureFormat_ASTC8x6Unorm;
        case Format::ASTC8x6_sRGB:          return WGPUTextureFormat_ASTC8x6UnormSrgb;
        case Format::ASTC8x8:               return WGPUTextureFormat_ASTC8x8Unorm;
        case Format::ASTC8x8_sRGB:          return WGPUTextureFormat_ASTC8x8UnormSrgb;
        case Format::ASTC10x5:              return WGPUTextureFormat_ASTC10x5Unorm;
        case Format::ASTC10x5_sRGB:         return WGPUTextureFormat_ASTC10x5UnormSrgb;
        case Format::ASTC10x6:              return WGPUTextureFormat_ASTC10x6Unorm;
        case Format::ASTC10x6_sRGB:         return WGPUTextureFormat_ASTC10x6UnormSrgb;
        case Format::ASTC10x8:              return WGPUTextureFormat_ASTC10x8Unorm;
        case Format::ASTC10x8_sRGB:         return WGPUTextureFormat_ASTC10x8UnormSrgb;
        case Format::ASTC10x10:             return WGPUTextureFormat_ASTC10x10Unorm;
        case Format::ASTC10x10_sRGB:        return WGPUTextureFormat_ASTC10x10UnormSrgb;
        case Format::ASTC12x10:             return WGPUTextureFormat_ASTC12x10Unorm;
        case Format::ASTC12x10_sRGB:        return WGPUTextureFormat_ASTC12x10UnormSrgb;
        case Format::ASTC12x12:             return WGPUTextureFormat_ASTC12x12Unorm;
        case Format::ASTC12x12_sRGB:        return WGPUTextureFormat_ASTC12x12UnormSrgb;

        /* --- ETC formats --- */
        case Format::ETC1UNorm:             return WGPUTextureFormat_ETC2RGB8Unorm; // Map ETC1 to ETC2 RGB8 format because it's compatible and more widely supported in WebGPU
        case Format::ETC2UNorm:             return WGPUTextureFormat_ETC2RGB8Unorm;
        case Format::ETC2UNorm_sRGB:        return WGPUTextureFormat_ETC2RGB8UnormSrgb;
        // Additional ETC variants (A1 / RGBA) mapping where available
        // case Format::ETC2RGBA8UNorm:     return WGPUTextureFormat_ETC2RGBA8Unorm;
        // case Format::ETC2RGBA8UNorm_sRGB: return WGPUTextureFormat_ETC2RGBA8UnormSrgb;

        default:                            break;
    }
    return WGPUTextureFormat_Undefined;
}

WGPUVertexFormat ToWGVertexFormat(const Format format)
{
    switch (format)
    {
        case Format::R8UInt:        return WGPUVertexFormat_Uint8;
        case Format::RG8UInt:       return WGPUVertexFormat_Uint8x2;
        case Format::RGBA8UInt:     return WGPUVertexFormat_Uint8x4;

        case Format::R8SInt:        return WGPUVertexFormat_Sint8;
        case Format::RG8SInt:       return WGPUVertexFormat_Sint8x2;
        case Format::RGBA8SInt:     return WGPUVertexFormat_Sint8x4;

        case Format::R8UNorm:       return WGPUVertexFormat_Unorm8;
        case Format::RG8UNorm:      return WGPUVertexFormat_Unorm8x2;
        case Format::RGBA8UNorm:    return WGPUVertexFormat_Unorm8x4;

        case Format::R8SNorm:       return WGPUVertexFormat_Snorm8;
        case Format::RG8SNorm:      return WGPUVertexFormat_Snorm8x2;
        case Format::RGBA8SNorm:    return WGPUVertexFormat_Snorm8x4;

        case Format::R16UInt:       return WGPUVertexFormat_Uint16;
        case Format::RG16UInt:      return WGPUVertexFormat_Uint16x2;
        case Format::RGBA16UInt:    return WGPUVertexFormat_Uint16x4;

        case Format::R16SInt:       return WGPUVertexFormat_Sint16;
        case Format::RG16SInt:      return WGPUVertexFormat_Sint16x2;
        case Format::RGBA16SInt:    return WGPUVertexFormat_Sint16x4;

        case Format::R16UNorm:      return WGPUVertexFormat_Unorm16;
        case Format::RG16UNorm:     return WGPUVertexFormat_Unorm16x2;
        case Format::RGBA16UNorm:   return WGPUVertexFormat_Unorm16x4;

        case Format::R16SNorm:      return WGPUVertexFormat_Snorm16;
        case Format::RG16SNorm:     return WGPUVertexFormat_Snorm16x2;
        case Format::RGBA16SNorm:   return WGPUVertexFormat_Snorm16x4;

        case Format::R16Float:      return WGPUVertexFormat_Float16;
        case Format::RG16Float:     return WGPUVertexFormat_Float16x2;
        case Format::RGBA16Float:   return WGPUVertexFormat_Float16x4;

        case Format::R32Float:      return WGPUVertexFormat_Float32;
        case Format::RG32Float:     return WGPUVertexFormat_Float32x2;
        case Format::RGB32Float:    return WGPUVertexFormat_Float32x3;
        case Format::RGBA32Float:   return WGPUVertexFormat_Float32x4;

        case Format::R32UInt:       return WGPUVertexFormat_Uint32;
        case Format::RG32UInt:      return WGPUVertexFormat_Uint32x2;
        case Format::RGB32UInt:     return WGPUVertexFormat_Uint32x3;
        case Format::RGBA32UInt:    return WGPUVertexFormat_Uint32x4;

        case Format::R32SInt:       return WGPUVertexFormat_Sint32;
        case Format::RG32SInt:      return WGPUVertexFormat_Sint32x2;
        case Format::RGB32SInt:     return WGPUVertexFormat_Sint32x3;
        case Format::RGBA32SInt:    return WGPUVertexFormat_Sint32x4;

        case Format::RGB10A2UNorm:  return WGPUVertexFormat_Unorm10_10_10_2;
        case Format::BGRA8UNorm:    return WGPUVertexFormat_Unorm8x4BGRA;

        default:                    break;
    }
    MapFailed("Format", "WGPUVertexFormat");
}

WGPUCullMode ToWGCullMode(const CullMode mode)
{
    switch (mode)
    {
        case CullMode::Disabled: return WGPUCullMode_None;
        case CullMode::Front:    return WGPUCullMode_Front;
        case CullMode::Back:     return WGPUCullMode_Back;
        default:                 break;
    }
    MapFailed("CullMode", "WGPUCullMode");
}

WGPULoadOp ToWGLoadOp(const AttachmentLoadOp loadOp)
{
    switch (loadOp)
    {
        case AttachmentLoadOp::Undefined:   return WGPULoadOp_Undefined;
        case AttachmentLoadOp::Load:        return WGPULoadOp_Load;
        case AttachmentLoadOp::Clear:       return WGPULoadOp_Clear;
        default:                            break;
    }
    MapFailed("AttachmentLoadOp", "WGPULoadOp");
}

WGPUStoreOp ToWGStoreOp(const AttachmentStoreOp storeOp)
{
    switch (storeOp)
    {
        case AttachmentStoreOp::Undefined:  return WGPUStoreOp_Discard;
        case AttachmentStoreOp::Store:      return WGPUStoreOp_Store;
        default:                            break;
    }
    MapFailed("AttachmentStoreOp", "WGPUStoreOp");
}

WGPUCompareFunction ToWGCompareFunc(const CompareOp compareOp)
{
    switch (compareOp)
    {
        case CompareOp::NeverPass:      return WGPUCompareFunction_Never;
        case CompareOp::Less:           return WGPUCompareFunction_Less;
        case CompareOp::Equal:          return WGPUCompareFunction_Equal;
        case CompareOp::LessEqual:      return WGPUCompareFunction_LessEqual;
        case CompareOp::Greater:        return WGPUCompareFunction_Greater;
        case CompareOp::NotEqual:       return WGPUCompareFunction_NotEqual;
        case CompareOp::GreaterEqual:   return WGPUCompareFunction_GreaterEqual;
        case CompareOp::AlwaysPass:     return WGPUCompareFunction_Always;
        default:                        break;
    }
    MapFailed("CompareOp", "WGPUCompareFunction");
}

WGPUStencilOperation ToWGStencilOperation(const StencilOp stencilOp)
{
    switch (stencilOp)
    {
        case StencilOp::Keep:       return WGPUStencilOperation_Keep;
        case StencilOp::Zero:       return WGPUStencilOperation_Zero;
        case StencilOp::Replace:    return WGPUStencilOperation_Replace;
        case StencilOp::IncClamp:   return WGPUStencilOperation_IncrementClamp;
        case StencilOp::DecClamp:   return WGPUStencilOperation_DecrementClamp;
        case StencilOp::Invert:     return WGPUStencilOperation_Invert;
        case StencilOp::IncWrap:    return WGPUStencilOperation_IncrementWrap;
        case StencilOp::DecWrap:    return WGPUStencilOperation_DecrementWrap;
    }
    MapFailed("StencilOp", "WGPUStencilOperation");
}

WGPUBlendFactor ToWGBlendFactor(const BlendOp blendOp)
{
    switch (blendOp)
    {
        case BlendOp::Zero:             return WGPUBlendFactor_Zero;
        case BlendOp::One:              return WGPUBlendFactor_One;
        case BlendOp::SrcColor:         return WGPUBlendFactor_Src;
        case BlendOp::InvSrcColor:      return WGPUBlendFactor_OneMinusSrc;
        case BlendOp::SrcAlpha:         return WGPUBlendFactor_SrcAlpha;
        case BlendOp::InvSrcAlpha:      return WGPUBlendFactor_OneMinusSrcAlpha;
        case BlendOp::DstColor:         return WGPUBlendFactor_Dst;
        case BlendOp::InvDstColor:      return WGPUBlendFactor_OneMinusDst;
        case BlendOp::DstAlpha:         return WGPUBlendFactor_DstAlpha;
        case BlendOp::InvDstAlpha:      return WGPUBlendFactor_OneMinusDstAlpha;
        case BlendOp::SrcAlphaSaturate: return WGPUBlendFactor_SrcAlphaSaturated;
        case BlendOp::BlendFactor:      return WGPUBlendFactor_Constant;
        case BlendOp::InvBlendFactor:   return WGPUBlendFactor_OneMinusConstant;
        case BlendOp::Src1Color:        return WGPUBlendFactor_Src1;
        case BlendOp::InvSrc1Color:     return WGPUBlendFactor_OneMinusSrc1;
        case BlendOp::Src1Alpha:        return WGPUBlendFactor_Src1Alpha;
        case BlendOp::InvSrc1Alpha:     return WGPUBlendFactor_OneMinusSrc1Alpha;
        default:                        break;
    }
    MapFailed("BlendOp", "WGPUBlendFactor");
}

WGPUBlendOperation ToWGBlendOperation(const BlendArithmetic blendArithmetic)
{
    switch (blendArithmetic)
    {
        case BlendArithmetic::Add:          return WGPUBlendOperation_Add;
        case BlendArithmetic::Subtract:     return WGPUBlendOperation_Subtract;
        case BlendArithmetic::RevSubtract:  return WGPUBlendOperation_ReverseSubtract;
        case BlendArithmetic::Min:          return WGPUBlendOperation_Min;
        case BlendArithmetic::Max:          return WGPUBlendOperation_Max;
        default:                            break;
    }
    MapFailed("BlendArithmetic", "WGPUBlendOperation");
}

WGPUPrimitiveTopology ToWGPrimitiveTopology(const PrimitiveTopology topology)
{
    switch (topology)
    {
        case PrimitiveTopology::PointList:      return WGPUPrimitiveTopology_PointList;
        case PrimitiveTopology::LineList:       return WGPUPrimitiveTopology_LineList;
        case PrimitiveTopology::LineStrip:      return WGPUPrimitiveTopology_LineStrip;
        case PrimitiveTopology::TriangleList:   return WGPUPrimitiveTopology_TriangleList;
        case PrimitiveTopology::TriangleStrip:  return WGPUPrimitiveTopology_TriangleStrip;
        default:                                break;
    }
    MapFailed("PrimitiveTopology", "WGPUPrimitiveTopology");
}

bool IsWGTextureFormatBC(WGPUTextureFormat format)
{
    switch (format)
    {
        case WGPUTextureFormat_BC1RGBAUnorm:
        case WGPUTextureFormat_BC1RGBAUnormSrgb:
        case WGPUTextureFormat_BC2RGBAUnorm:
        case WGPUTextureFormat_BC2RGBAUnormSrgb:
        case WGPUTextureFormat_BC3RGBAUnorm:
        case WGPUTextureFormat_BC3RGBAUnormSrgb:
        case WGPUTextureFormat_BC4RUnorm:
        case WGPUTextureFormat_BC4RSnorm:
        case WGPUTextureFormat_BC5RGUnorm:
        case WGPUTextureFormat_BC5RGSnorm:
            return true;
        default:
            return false;
    }
}

bool IsWGTextureFormatASTC(WGPUTextureFormat format)
{
    switch (format)
    {
        case WGPUTextureFormat_ASTC4x4Unorm:
        case WGPUTextureFormat_ASTC4x4UnormSrgb:
        case WGPUTextureFormat_ASTC5x4Unorm:
        case WGPUTextureFormat_ASTC5x4UnormSrgb:
        case WGPUTextureFormat_ASTC5x5Unorm:
        case WGPUTextureFormat_ASTC5x5UnormSrgb:
        case WGPUTextureFormat_ASTC6x5Unorm:
        case WGPUTextureFormat_ASTC6x5UnormSrgb:
        case WGPUTextureFormat_ASTC6x6Unorm:
        case WGPUTextureFormat_ASTC6x6UnormSrgb:
        case WGPUTextureFormat_ASTC8x5Unorm:
        case WGPUTextureFormat_ASTC8x5UnormSrgb:
        case WGPUTextureFormat_ASTC8x6Unorm:
        case WGPUTextureFormat_ASTC8x6UnormSrgb:
        case WGPUTextureFormat_ASTC8x8Unorm:
        case WGPUTextureFormat_ASTC8x8UnormSrgb:
        case WGPUTextureFormat_ASTC10x5Unorm:
        case WGPUTextureFormat_ASTC10x5UnormSrgb:
        case WGPUTextureFormat_ASTC10x6Unorm:
        case WGPUTextureFormat_ASTC10x6UnormSrgb:
        case WGPUTextureFormat_ASTC10x8Unorm:
        case WGPUTextureFormat_ASTC10x8UnormSrgb:
        case WGPUTextureFormat_ASTC10x10Unorm:
        case WGPUTextureFormat_ASTC10x10UnormSrgb:
        case WGPUTextureFormat_ASTC12x10Unorm:
        case WGPUTextureFormat_ASTC12x10UnormSrgb:
        case WGPUTextureFormat_ASTC12x12Unorm:
        case WGPUTextureFormat_ASTC12x12UnormSrgb:
            return true;
        default:
            return false;
    }
}

bool IsWGTextureFormatETC2(WGPUTextureFormat format)
{
    switch (format)
    {
        case WGPUTextureFormat_ETC2RGB8Unorm:
        case WGPUTextureFormat_ETC2RGB8UnormSrgb:
        case WGPUTextureFormat_ETC2RGBA8Unorm:      // may not be used, include if available
        case WGPUTextureFormat_ETC2RGBA8UnormSrgb:  // may not be used, include if available
            return true;
        default:
            return false;
    }
}


} // /namespace WGTypes

} // /namespace LLGL



// ================================================================================
