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
