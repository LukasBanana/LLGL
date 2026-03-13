/*
 * D3D9Types.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D9Types.h"
#include "../DXCommon/DXCore.h"
#include "../../Core/Exception.h"
#include <stdexcept>
#include <string>


#define LLGL_TRAP_D3D_MAP(TYPE, VALUE, D3DTYPE) \
    LLGL_TRAP("failed to map LLGL::%s(%d) to %s Direct3D parameter", #TYPE, static_cast<int>(VALUE), #D3DTYPE)


namespace LLGL
{

namespace D3D9Types
{


D3DFORMAT ToD3DFormat(const Format format)
{
    switch (format)
    {
        case Format::Undefined:         return D3DFMT_UNKNOWN;

        /* --- Alpha channel color formats --- */
        case Format::A8UNorm:           return D3DFMT_A8;

        /* --- Red channel color formats --- */
        case Format::R8UNorm:           return D3DFMT_L8;
        case Format::R8SNorm:           break;
        case Format::R8UInt:            break;
        case Format::R8SInt:            break;

        case Format::R16UNorm:          return D3DFMT_L16;
        case Format::R16SNorm:          break;
        case Format::R16UInt:           break;
        case Format::R16SInt:           break;
        case Format::R16Float:          return D3DFMT_R16F;

        case Format::R32UInt:           break;
        case Format::R32SInt:           break;
        case Format::R32Float:          return D3DFMT_R32F;

        case Format::R64Float:          break;

        /* --- RG color formats --- */
        case Format::RG8UNorm:          break;
        case Format::RG8SNorm:          break;
        case Format::RG8UInt:           break;
        case Format::RG8SInt:           break;

        case Format::RG16UNorm:         break;
        case Format::RG16SNorm:         break;
        case Format::RG16UInt:          break;
        case Format::RG16SInt:          break;
        case Format::RG16Float:         break;

        case Format::RG32UInt:          break;
        case Format::RG32SInt:          break;
        case Format::RG32Float:         return D3DFMT_G32R32F; // Flipped

        case Format::RG64Float:         break;

        /* --- RGB color formats --- */
        case Format::RGB8UNorm:         return D3DFMT_X8R8G8B8;
        case Format::RGB8UNorm_sRGB:    break;
        case Format::RGB8SNorm:         break;
        case Format::RGB8UInt:          break;
        case Format::RGB8SInt:          break;

        case Format::RGB16UNorm:        break;
        case Format::RGB16SNorm:        break;
        case Format::RGB16UInt:         break;
        case Format::RGB16SInt:         break;
        case Format::RGB16Float:        break;

        case Format::RGB32UInt:         break;
        case Format::RGB32SInt:         break;
        case Format::RGB32Float:        break;

        case Format::RGB64Float:        break;

        /* --- RGBA color formats --- */
        case Format::RGBA8UNorm:        return D3DFMT_A8R8G8B8;
        case Format::RGBA8UNorm_sRGB:   break;
        case Format::RGBA8SNorm:        break;
        case Format::RGBA8UInt:         break;
        case Format::RGBA8SInt:         break;

        case Format::RGBA16UNorm:       return D3DFMT_A16B16G16R16;
        case Format::RGBA16SNorm:       break;
        case Format::RGBA16UInt:        break;
        case Format::RGBA16SInt:        break;
        case Format::RGBA16Float:       return D3DFMT_A16B16G16R16F;

        case Format::RGBA32UInt:        break;
        case Format::RGBA32SInt:        break;
        case Format::RGBA32Float:       return D3DFMT_A32B32G32R32F;

        case Format::RGBA64Float:       break;

        /* --- BGRA color formats --- */
        case Format::BGRA8UNorm:        break;
        case Format::BGRA8UNorm_sRGB:   break;
        case Format::BGRA8SNorm:        break;
        case Format::BGRA8UInt:         break;
        case Format::BGRA8SInt:         break;

        /* --- Packed formats --- */
        case Format::RGB10A2UNorm:      break;
        case Format::RGB10A2UInt:       break;
        case Format::RG11B10Float:      break;
        case Format::RGB9E5Float:       break;

        /* --- Depth-stencil formats --- */
        case Format::D16UNorm:          return D3DFMT_D16_LOCKABLE;
        case Format::D32Float:          return D3DFMT_D32;
        case Format::D24UNormS8UInt:    return D3DFMT_D24S8;
        case Format::D32FloatS8X24UInt: break;

        /* --- Block compression (BC) formats --- */
        case Format::BC1UNorm:          return D3DFMT_DXT1;
        case Format::BC1UNorm_sRGB:     break;
        case Format::BC2UNorm:          return D3DFMT_DXT2;
        case Format::BC2UNorm_sRGB:     break;
        case Format::BC3UNorm:          return D3DFMT_DXT3;
        case Format::BC3UNorm_sRGB:     break;
        case Format::BC4UNorm:          return D3DFMT_DXT4;
        case Format::BC4SNorm:          break;
        case Format::BC5UNorm:          return D3DFMT_DXT5;
        case Format::BC5SNorm:          break;

        /* --- Advanced scalable texture compression (ASTC) formats --- */
        case Format::ASTC4x4:           break;
        case Format::ASTC4x4_sRGB:      break;
        case Format::ASTC5x4:           break;
        case Format::ASTC5x4_sRGB:      break;
        case Format::ASTC5x5:           break;
        case Format::ASTC5x5_sRGB:      break;
        case Format::ASTC6x5:           break;
        case Format::ASTC6x5_sRGB:      break;
        case Format::ASTC6x6:           break;
        case Format::ASTC6x6_sRGB:      break;
        case Format::ASTC8x5:           break;
        case Format::ASTC8x5_sRGB:      break;
        case Format::ASTC8x6:           break;
        case Format::ASTC8x6_sRGB:      break;
        case Format::ASTC8x8:           break;
        case Format::ASTC8x8_sRGB:      break;
        case Format::ASTC10x5:          break;
        case Format::ASTC10x5_sRGB:     break;
        case Format::ASTC10x6:          break;
        case Format::ASTC10x6_sRGB:     break;
        case Format::ASTC10x8:          break;
        case Format::ASTC10x8_sRGB:     break;
        case Format::ASTC10x10:         break;
        case Format::ASTC10x10_sRGB:    break;
        case Format::ASTC12x10:         break;
        case Format::ASTC12x10_sRGB:    break;
        case Format::ASTC12x12:         break;
        case Format::ASTC12x12_sRGB:    break;

        /* --- Ericsson texture compression (ETC) formats --- */
        case Format::ETC1UNorm:         break;
        case Format::ETC2UNorm:         break;
        case Format::ETC2UNorm_sRGB:    break;
    }
    LLGL_TRAP_D3D_MAP(Format, format, D3DFORMAT);
}

D3DFORMAT ToD3DIndexFormat(const Format format)
{
    switch (format)
    {
        case Format::R16UInt:   return D3DFMT_INDEX16;
        case Format::R32UInt:   return D3DFMT_INDEX32;
        default:                break;
    }
    LLGL_TRAP_D3D_MAP(Format, format, D3DFORMAT);
}

D3DDECLTYPE ToD3DDeclType(const Format format)
{
    switch (format)
    {
        case Format::R32Float:      return D3DDECLTYPE_FLOAT1;
        case Format::RG32Float:     return D3DDECLTYPE_FLOAT2;
        case Format::RGB32Float:    return D3DDECLTYPE_FLOAT3;
        case Format::RGBA32Float:   return D3DDECLTYPE_FLOAT4;
        case Format::RGBA8UInt:     return D3DDECLTYPE_UBYTE4;
        case Format::RG16SInt:      return D3DDECLTYPE_SHORT2;
        case Format::RGBA16SInt:    return D3DDECLTYPE_SHORT4;
        case Format::RGBA8UNorm:    return D3DDECLTYPE_UBYTE4N;
        case Format::RG16SNorm:     return D3DDECLTYPE_SHORT2N;
        case Format::RGBA16SNorm:   return D3DDECLTYPE_SHORT4N;
        case Format::RG16UNorm:     return D3DDECLTYPE_USHORT2N;
        case Format::RGBA16UNorm:   return D3DDECLTYPE_USHORT4N;
        case Format::RGB10A2UInt:   return D3DDECLTYPE_UDEC3;
        case Format::RGB10A2UNorm:  return D3DDECLTYPE_DEC3N;
        case Format::RG16Float:     return D3DDECLTYPE_FLOAT16_2;
        case Format::RGBA16Float:   return D3DDECLTYPE_FLOAT16_4;
        default:                    break;
    }
    LLGL_TRAP_D3D_MAP(Format, format, D3DFORMAT);
}

D3DTEXTUREADDRESS ToD3DTextureAddress(const SamplerAddressMode mode)
{
    switch (mode)
    {
        case SamplerAddressMode::Repeat:        return D3DTADDRESS_WRAP;
        case SamplerAddressMode::Mirror:        return D3DTADDRESS_MIRROR;
        case SamplerAddressMode::Clamp:         return D3DTADDRESS_CLAMP;
        case SamplerAddressMode::Border:        return D3DTADDRESS_BORDER;
        case SamplerAddressMode::MirrorOnce:    return D3DTADDRESS_MIRRORONCE;
    }
    LLGL_TRAP_D3D_MAP(SamplerAddressMode, mode, D3DTEXTUREADDRESS);
}

D3DCOLOR ToD3DColor(const float color[4])
{
    return D3DCOLOR_COLORVALUE(color[0], color[1], color[2], color[3]);
}

D3DTEXTUREFILTERTYPE ToD3DTextureFilter(const SamplerFilter type)
{
    switch (type)
    {
        case SamplerFilter::Nearest:    return D3DTEXF_POINT;
        case SamplerFilter::Linear:     return D3DTEXF_LINEAR;
    }
    LLGL_TRAP_D3D_MAP(SamplerFilter, type, D3DTEXTUREFILTERTYPE);
}

D3DCMPFUNC ToD3DCmpFunc( const CompareOp op )
{
    switch (op)
    {
        case CompareOp::NeverPass:      return D3DCMP_NEVER;
        case CompareOp::Less:           return D3DCMP_LESS;
        case CompareOp::Equal:          return D3DCMP_EQUAL;
        case CompareOp::LessEqual:      return D3DCMP_LESSEQUAL;
        case CompareOp::Greater:        return D3DCMP_GREATER;
        case CompareOp::NotEqual:       return D3DCMP_NOTEQUAL;
        case CompareOp::GreaterEqual:   return D3DCMP_GREATEREQUAL;
        case CompareOp::AlwaysPass:     return D3DCMP_ALWAYS;
    }
    LLGL_TRAP_D3D_MAP(CompareOp, op, D3DCMPFUNC);
}

D3DSTENCILOP ToD3DStenciOp( const StencilOp op )
{
    switch (op)
    {
        case StencilOp::Keep:       return D3DSTENCILOP_KEEP;
        case StencilOp::Zero:       return D3DSTENCILOP_ZERO;
        case StencilOp::Replace:    return D3DSTENCILOP_REPLACE;
        case StencilOp::IncClamp:   return D3DSTENCILOP_INCRSAT;
        case StencilOp::DecClamp:   return D3DSTENCILOP_DECRSAT;
        case StencilOp::Invert:     return D3DSTENCILOP_INVERT;
        case StencilOp::IncWrap:    return D3DSTENCILOP_INCR;
        case StencilOp::DecWrap:    return D3DSTENCILOP_DECR;
    }
    LLGL_TRAP_D3D_MAP(StencilOp, op, D3DSTENCILOP);
}

D3DCULL ToD3DCull( const CullMode mode )
{
    switch (mode)
    {
        case CullMode::Disabled:    return D3DCULL_NONE;
        case CullMode::Front:       return D3DCULL_CW;
        case CullMode::Back:        return D3DCULL_CCW;
    }
    LLGL_TRAP_D3D_MAP(CullMode, mode, D3DCULL);
}

D3DFILLMODE ToD3DFillMode( const PolygonMode mode )
{
    switch (mode)
    {
        case PolygonMode::Fill:       return D3DFILL_SOLID;
        case PolygonMode::Wireframe:  return D3DFILL_WIREFRAME;
        case PolygonMode::Points:     return D3DFILL_POINT;
    }
    LLGL_TRAP_D3D_MAP(PolygonMode, mode, D3DFILLMODE);
}


D3DBLENDOP ToD3DBlendOp( const BlendArithmetic arithmetic )
{
    switch (arithmetic)
    {
        case BlendArithmetic::Add:          return D3DBLENDOP_ADD;
        case BlendArithmetic::Subtract:     return D3DBLENDOP_SUBTRACT;
        case BlendArithmetic::RevSubtract:  return D3DBLENDOP_REVSUBTRACT;
        case BlendArithmetic::Min:          return D3DBLENDOP_MIN;
        case BlendArithmetic::Max:          return D3DBLENDOP_MAX;
    }
    LLGL_TRAP_D3D_MAP(BlendArithmetic, arithmetic, D3DBLENDOP);
}

D3DBLEND ToD3DBlend( const BlendOp op )
{
    switch (op)
    {
        case BlendOp::Zero:             return D3DBLEND_ZERO;
        case BlendOp::One:              return D3DBLEND_ONE;
        case BlendOp::SrcColor:         return D3DBLEND_SRCCOLOR;
        case BlendOp::InvSrcColor:      return D3DBLEND_INVSRCCOLOR;
        case BlendOp::SrcAlpha:         return D3DBLEND_SRCALPHA;
        case BlendOp::InvSrcAlpha:      return D3DBLEND_INVSRCALPHA;
        case BlendOp::DstColor:         return D3DBLEND_DESTCOLOR;
        case BlendOp::InvDstColor:      return D3DBLEND_INVDESTCOLOR;
        case BlendOp::DstAlpha:         return D3DBLEND_DESTALPHA;
        case BlendOp::InvDstAlpha:      return D3DBLEND_INVDESTALPHA;
        case BlendOp::SrcAlphaSaturate: return D3DBLEND_SRCALPHASAT;
        case BlendOp::BlendFactor:      return D3DBLEND_BLENDFACTOR;
        case BlendOp::InvBlendFactor:   return D3DBLEND_INVBLENDFACTOR;
        case BlendOp::Src1Color:        return D3DBLEND_SRCCOLOR2;
        case BlendOp::InvSrc1Color:     return D3DBLEND_INVSRCCOLOR2;
        case BlendOp::Src1Alpha:        break; // Not supported
        case BlendOp::InvSrc1Alpha:     break; // Not supported
    }
    LLGL_TRAP_D3D_MAP(BlendOp, op, D3DBLEND);
}


Format ToFormat(const D3DFORMAT format)
{
    switch (format)
    {
        case D3DFMT_UNKNOWN:        return Format::Undefined;

        /* --- Alpha channel color formats --- */
        case D3DFMT_A8:             return Format::A8UNorm;

        /* --- Red channel color formats --- */
        case D3DFMT_L8:             return Format::R8UNorm;

        case D3DFMT_L16:            return Format::R16UNorm;
        case D3DFMT_R16F:           return Format::R16Float;

        case D3DFMT_R32F:           return Format::R32Float;

        /* --- RG color formats --- */
        case D3DFMT_G32R32F:        return Format::RG32Float; // Flipped

        /* --- RGB color formats --- */
        case D3DFMT_X8R8G8B8:       return Format::RGB8UNorm;

        /* --- RGBA color formats --- */
        case D3DFMT_A8R8G8B8:       return Format::RGBA8UNorm;

        case D3DFMT_A16B16G16R16:   return Format::RGBA16UNorm;
        case D3DFMT_A16B16G16R16F:  return Format::RGBA16Float;

        case D3DFMT_A32B32G32R32F:  return Format::RGBA32Float;

        /* --- Depth-stencil formats --- */
        case D3DFMT_D16_LOCKABLE:   return Format::D16UNorm;
        case D3DFMT_D32:            return Format::D32Float;
        case D3DFMT_D24S8:          return Format::D24UNormS8UInt;

        /* --- Block compression (BC) formats --- */
        case D3DFMT_DXT1:           return Format::BC1UNorm;
        case D3DFMT_DXT2:           return Format::BC2UNorm;
        case D3DFMT_DXT3:           return Format::BC3UNorm;
        case D3DFMT_DXT4:           return Format::BC4UNorm;
        case D3DFMT_DXT5:           return Format::BC5UNorm;

        /* --- Index formats --- */
        case D3DFMT_INDEX16:        return Format::R16UInt;
        case D3DFMT_INDEX32:        return Format::R32UInt;
    }
    return Format::Undefined;
}

D3DPRIMITIVETYPE ToD3DPrimitiveType(const PrimitiveTopology topology)
{
    switch (topology)
    {
        case PrimitiveTopology::PointList:      return D3DPT_POINTLIST;
        case PrimitiveTopology::LineList:       return D3DPT_LINELIST;
        case PrimitiveTopology::LineStrip:      return D3DPT_LINESTRIP;
        case PrimitiveTopology::TriangleList:   return D3DPT_TRIANGLELIST;
        case PrimitiveTopology::TriangleStrip:  return D3DPT_TRIANGLESTRIP;
    }
    LLGL_TRAP_D3D_MAP(PrimitiveTopology, topology, D3DPRIMITIVETYPE);
}


} // /namespace D3D9Types

} // /namespace LLGL



// ================================================================================
