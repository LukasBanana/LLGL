/*
 * DXTypes.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "DXTypes.h"
#include <stdexcept>
#include <string>


namespace LLGL
{

namespace DXTypes
{


[[noreturn]]
void MapFailed(const char* typeName, const char* dxTypeName)
{
    throw std::invalid_argument(
        "failed to map <LLGL::" + std::string(typeName) + "> to <" + std::string(dxTypeName) + "> Direct3D parameter"
    );
}

[[noreturn]]
void UnmapFailed(const char* typeName, const char* dxTypeName)
{
    throw std::invalid_argument(
        "failed to unmap <LLGL::" + std::string(typeName) + "> from <" + std::string(dxTypeName) + "> Direct3D parameter"
    );
}

[[noreturn]]
void ParamNotSupported(const char* paramName, const char* requirement)
{
    throw std::runtime_error(
        "parameter '" + std::string(paramName) + "' requires " + std::string(requirement)
    );
}

DXGI_FORMAT Map(const DataType dataType)
{
    switch (dataType)
    {
        case DataType::Int8:    return DXGI_FORMAT_R8_SINT;
        case DataType::UInt8:   return DXGI_FORMAT_R8_UINT;
        case DataType::Int16:   return DXGI_FORMAT_R16_SINT;
        case DataType::UInt16:  return DXGI_FORMAT_R16_UINT;
        case DataType::Int32:   return DXGI_FORMAT_R32_SINT;
        case DataType::UInt32:  return DXGI_FORMAT_R32_UINT;
        case DataType::Float16: return DXGI_FORMAT_R16_FLOAT;
        case DataType::Float32: return DXGI_FORMAT_R32_FLOAT;
        case DataType::Float64: break;
    }
    MapFailed("DataType", "DXGI_FORMAT");
}

DXGI_FORMAT Map(const Format format)
{
    switch (format)
    {
        case Format::Undefined:         break;

        /* --- Alpha channel color formats --- */
        case Format::A8UNorm:           return DXGI_FORMAT_A8_UNORM;

        /* --- Red channel color formats --- */
        case Format::R8UNorm:           return DXGI_FORMAT_R8_UNORM;
        case Format::R8SNorm:           return DXGI_FORMAT_R8_SNORM;
        case Format::R8UInt:            return DXGI_FORMAT_R8_UINT;
        case Format::R8SInt:            return DXGI_FORMAT_R8_SINT;

        case Format::R16UNorm:          return DXGI_FORMAT_R16_UNORM;
        case Format::R16SNorm:          return DXGI_FORMAT_R16_SNORM;
        case Format::R16UInt:           return DXGI_FORMAT_R16_UINT;
        case Format::R16SInt:           return DXGI_FORMAT_R16_SINT;
        case Format::R16Float:          return DXGI_FORMAT_R16_FLOAT;

        case Format::R32UInt:           return DXGI_FORMAT_R32_UINT;
        case Format::R32SInt:           return DXGI_FORMAT_R32_SINT;
        case Format::R32Float:          return DXGI_FORMAT_R32_FLOAT;

        case Format::R64Float:          break;

        /* --- RG color formats --- */
        case Format::RG8UNorm:          return DXGI_FORMAT_R8G8_UNORM;
        case Format::RG8SNorm:          return DXGI_FORMAT_R8G8_SNORM;
        case Format::RG8UInt:           return DXGI_FORMAT_R8G8_UINT;
        case Format::RG8SInt:           return DXGI_FORMAT_R8G8_SINT;

        case Format::RG16UNorm:         return DXGI_FORMAT_R16G16_UNORM;
        case Format::RG16SNorm:         return DXGI_FORMAT_R16G16_SNORM;
        case Format::RG16UInt:          return DXGI_FORMAT_R16G16_UINT;
        case Format::RG16SInt:          return DXGI_FORMAT_R16G16_SINT;
        case Format::RG16Float:         return DXGI_FORMAT_R16G16_FLOAT;

        case Format::RG32UInt:          return DXGI_FORMAT_R32G32_UINT;
        case Format::RG32SInt:          return DXGI_FORMAT_R32G32_SINT;
        case Format::RG32Float:         return DXGI_FORMAT_R32G32_FLOAT;

        case Format::RG64Float:         break;

        /* --- RGB color formats --- */
        case Format::RGB8UNorm:         break;
        case Format::RGB8UNorm_sRGB:    break;
        case Format::RGB8SNorm:         break;
        case Format::RGB8UInt:          break;
        case Format::RGB8SInt:          break;

        case Format::RGB16UNorm:        break;
        case Format::RGB16SNorm:        break;
        case Format::RGB16UInt:         break;
        case Format::RGB16SInt:         break;
        case Format::RGB16Float:        break;

        case Format::RGB32UInt:         return DXGI_FORMAT_R32G32B32_UINT;
        case Format::RGB32SInt:         return DXGI_FORMAT_R32G32B32_SINT;
        case Format::RGB32Float:        return DXGI_FORMAT_R32G32B32_FLOAT;

        case Format::RGB64Float:        break;

        /* --- RGBA color formats --- */
        case Format::RGBA8UNorm:        return DXGI_FORMAT_R8G8B8A8_UNORM;
        case Format::RGBA8UNorm_sRGB:   return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
        case Format::RGBA8SNorm:        return DXGI_FORMAT_R8G8B8A8_SNORM;
        case Format::RGBA8UInt:         return DXGI_FORMAT_R8G8B8A8_UINT;
        case Format::RGBA8SInt:         return DXGI_FORMAT_R8G8B8A8_SINT;

        case Format::RGBA16UNorm:       return DXGI_FORMAT_R16G16B16A16_UNORM;
        case Format::RGBA16SNorm:       return DXGI_FORMAT_R16G16B16A16_SNORM;
        case Format::RGBA16UInt:        return DXGI_FORMAT_R16G16B16A16_UINT;
        case Format::RGBA16SInt:        return DXGI_FORMAT_R16G16B16A16_SINT;
        case Format::RGBA16Float:       return DXGI_FORMAT_R16G16B16A16_FLOAT;

        case Format::RGBA32UInt:        return DXGI_FORMAT_R32G32B32A32_UINT;
        case Format::RGBA32SInt:        return DXGI_FORMAT_R32G32B32A32_SINT;
        case Format::RGBA32Float:       return DXGI_FORMAT_R32G32B32A32_FLOAT;

        case Format::RGBA64Float:       break;

        /* --- BGRA color formats --- */
        case Format::BGRA8UNorm:        return DXGI_FORMAT_B8G8R8A8_UNORM;
        case Format::BGRA8UNorm_sRGB:   return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
        case Format::BGRA8SNorm:        break;
        case Format::BGRA8UInt:         break;
        case Format::BGRA8SInt:         break;

        /* --- Depth-stencil formats --- */
        case Format::D16UNorm:          return DXGI_FORMAT_R16_TYPELESS;
        case Format::D32Float:          return DXGI_FORMAT_R32_TYPELESS;
        case Format::D24UNormS8UInt:    return DXGI_FORMAT_R24G8_TYPELESS;
        case Format::D32FloatS8X24UInt: return DXGI_FORMAT_R32G8X24_TYPELESS;

        /* --- Compressed color formats --- */
        case Format::BC1RGB:            break;
        case Format::BC1RGBA:           return DXGI_FORMAT_BC1_UNORM;
        case Format::BC2RGBA:           return DXGI_FORMAT_BC2_UNORM;
        case Format::BC3RGBA:           return DXGI_FORMAT_BC3_UNORM;
    }
    MapFailed("Format", "DXGI_FORMAT");
}

D3D_PRIMITIVE_TOPOLOGY Map(const PrimitiveTopology topology)
{
    switch (topology)
    {
        case PrimitiveTopology::PointList:              return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
        case PrimitiveTopology::LineList:               return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
        case PrimitiveTopology::LineStrip:              return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
        case PrimitiveTopology::LineLoop:               break;
        case PrimitiveTopology::LineListAdjacency:      return D3D_PRIMITIVE_TOPOLOGY_LINELIST_ADJ;
        case PrimitiveTopology::LineStripAdjacency:     return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ;
        case PrimitiveTopology::TriangleList:           return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        case PrimitiveTopology::TriangleStrip:          return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
        case PrimitiveTopology::TriangleFan:            break;
        case PrimitiveTopology::TriangleListAdjacency:  return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ;
        case PrimitiveTopology::TriangleStripAdjacency: return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ;
        case PrimitiveTopology::Patches1:               return D3D_PRIMITIVE_TOPOLOGY_1_CONTROL_POINT_PATCHLIST;
        case PrimitiveTopology::Patches2:               return D3D_PRIMITIVE_TOPOLOGY_2_CONTROL_POINT_PATCHLIST;
        case PrimitiveTopology::Patches3:               return D3D_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST;
        case PrimitiveTopology::Patches4:               return D3D_PRIMITIVE_TOPOLOGY_4_CONTROL_POINT_PATCHLIST;
        case PrimitiveTopology::Patches5:               return D3D_PRIMITIVE_TOPOLOGY_5_CONTROL_POINT_PATCHLIST;
        case PrimitiveTopology::Patches6:               return D3D_PRIMITIVE_TOPOLOGY_6_CONTROL_POINT_PATCHLIST;
        case PrimitiveTopology::Patches7:               return D3D_PRIMITIVE_TOPOLOGY_7_CONTROL_POINT_PATCHLIST;
        case PrimitiveTopology::Patches8:               return D3D_PRIMITIVE_TOPOLOGY_8_CONTROL_POINT_PATCHLIST;
        case PrimitiveTopology::Patches9:               return D3D_PRIMITIVE_TOPOLOGY_9_CONTROL_POINT_PATCHLIST;
        case PrimitiveTopology::Patches10:              return D3D_PRIMITIVE_TOPOLOGY_10_CONTROL_POINT_PATCHLIST;
        case PrimitiveTopology::Patches11:              return D3D_PRIMITIVE_TOPOLOGY_11_CONTROL_POINT_PATCHLIST;
        case PrimitiveTopology::Patches12:              return D3D_PRIMITIVE_TOPOLOGY_12_CONTROL_POINT_PATCHLIST;
        case PrimitiveTopology::Patches13:              return D3D_PRIMITIVE_TOPOLOGY_13_CONTROL_POINT_PATCHLIST;
        case PrimitiveTopology::Patches14:              return D3D_PRIMITIVE_TOPOLOGY_14_CONTROL_POINT_PATCHLIST;
        case PrimitiveTopology::Patches15:              return D3D_PRIMITIVE_TOPOLOGY_15_CONTROL_POINT_PATCHLIST;
        case PrimitiveTopology::Patches16:              return D3D_PRIMITIVE_TOPOLOGY_16_CONTROL_POINT_PATCHLIST;
        case PrimitiveTopology::Patches17:              return D3D_PRIMITIVE_TOPOLOGY_17_CONTROL_POINT_PATCHLIST;
        case PrimitiveTopology::Patches18:              return D3D_PRIMITIVE_TOPOLOGY_18_CONTROL_POINT_PATCHLIST;
        case PrimitiveTopology::Patches19:              return D3D_PRIMITIVE_TOPOLOGY_19_CONTROL_POINT_PATCHLIST;
        case PrimitiveTopology::Patches20:              return D3D_PRIMITIVE_TOPOLOGY_20_CONTROL_POINT_PATCHLIST;
        case PrimitiveTopology::Patches21:              return D3D_PRIMITIVE_TOPOLOGY_21_CONTROL_POINT_PATCHLIST;
        case PrimitiveTopology::Patches22:              return D3D_PRIMITIVE_TOPOLOGY_22_CONTROL_POINT_PATCHLIST;
        case PrimitiveTopology::Patches23:              return D3D_PRIMITIVE_TOPOLOGY_23_CONTROL_POINT_PATCHLIST;
        case PrimitiveTopology::Patches24:              return D3D_PRIMITIVE_TOPOLOGY_24_CONTROL_POINT_PATCHLIST;
        case PrimitiveTopology::Patches25:              return D3D_PRIMITIVE_TOPOLOGY_25_CONTROL_POINT_PATCHLIST;
        case PrimitiveTopology::Patches26:              return D3D_PRIMITIVE_TOPOLOGY_26_CONTROL_POINT_PATCHLIST;
        case PrimitiveTopology::Patches27:              return D3D_PRIMITIVE_TOPOLOGY_27_CONTROL_POINT_PATCHLIST;
        case PrimitiveTopology::Patches28:              return D3D_PRIMITIVE_TOPOLOGY_28_CONTROL_POINT_PATCHLIST;
        case PrimitiveTopology::Patches29:              return D3D_PRIMITIVE_TOPOLOGY_29_CONTROL_POINT_PATCHLIST;
        case PrimitiveTopology::Patches30:              return D3D_PRIMITIVE_TOPOLOGY_30_CONTROL_POINT_PATCHLIST;
        case PrimitiveTopology::Patches31:              return D3D_PRIMITIVE_TOPOLOGY_31_CONTROL_POINT_PATCHLIST;
        case PrimitiveTopology::Patches32:              return D3D_PRIMITIVE_TOPOLOGY_32_CONTROL_POINT_PATCHLIST;
    }
    MapFailed("PrimitiveTopology", "D3D_PRIMITIVE_TOPOLOGY");
}

Format Unmap(const DXGI_FORMAT format)
{
    switch (format)
    {
        /* --- Alpha channel color formats --- */
        case DXGI_FORMAT_A8_UNORM:                  return Format::A8UNorm;

        /* --- Red channel color formats --- */
        case DXGI_FORMAT_R8_UNORM:                  return Format::R8UNorm;
        case DXGI_FORMAT_R8_SNORM:                  return Format::R8SNorm;
        case DXGI_FORMAT_R8_UINT:                   return Format::R8UInt;
        case DXGI_FORMAT_R8_SINT:                   return Format::R8SInt;

        case DXGI_FORMAT_R16_UNORM:                 return Format::R16UNorm;
        case DXGI_FORMAT_R16_SNORM:                 return Format::R16SNorm;
        case DXGI_FORMAT_R16_UINT:                  return Format::R16UInt;
        case DXGI_FORMAT_R16_SINT:                  return Format::R16SInt;
        case DXGI_FORMAT_R16_FLOAT:                 return Format::R16Float;

        case DXGI_FORMAT_R32_UINT:                  return Format::R32UInt;
        case DXGI_FORMAT_R32_SINT:                  return Format::R32SInt;
        case DXGI_FORMAT_R32_FLOAT:                 return Format::R32Float;

        /* --- RG color formats --- */
        case DXGI_FORMAT_R8G8_UNORM:                return Format::RG8UNorm;
        case DXGI_FORMAT_R8G8_SNORM:                return Format::RG8SNorm;
        case DXGI_FORMAT_R8G8_UINT:                 return Format::RG8UInt;
        case DXGI_FORMAT_R8G8_SINT:                 return Format::RG8SInt;

        case DXGI_FORMAT_R16G16_UNORM:              return Format::RG16UNorm;
        case DXGI_FORMAT_R16G16_SNORM:              return Format::RG16SNorm;
        case DXGI_FORMAT_R16G16_UINT:               return Format::RG16UInt;
        case DXGI_FORMAT_R16G16_SINT:               return Format::RG16SInt;
        case DXGI_FORMAT_R16G16_FLOAT:              return Format::RG16Float;

        case DXGI_FORMAT_R32G32_UINT:               return Format::RG32UInt;
        case DXGI_FORMAT_R32G32_SINT:               return Format::RG32SInt;
        case DXGI_FORMAT_R32G32_FLOAT:              return Format::RG32Float;

        /* --- RGB color formats --- */
        case DXGI_FORMAT_R32G32B32_UINT:            return Format::RGB32UInt;
        case DXGI_FORMAT_R32G32B32_SINT:            return Format::RGB32SInt;
        case DXGI_FORMAT_R32G32B32_FLOAT:           return Format::RGB32Float;

        /* --- RGBA color formats --- */
        case DXGI_FORMAT_R8G8B8A8_UNORM:            return Format::RGBA8UNorm;
        case DXGI_FORMAT_R8G8B8A8_SNORM:            return Format::RGBA8SNorm;
        case DXGI_FORMAT_R8G8B8A8_UINT:             return Format::RGBA8UInt;
        case DXGI_FORMAT_R8G8B8A8_SINT:             return Format::RGBA8SInt;

        case DXGI_FORMAT_R16G16B16A16_UNORM:        return Format::RGBA16UNorm;
        case DXGI_FORMAT_R16G16B16A16_SNORM:        return Format::RGBA16SNorm;
        case DXGI_FORMAT_R16G16B16A16_UINT:         return Format::RGBA16UInt;
        case DXGI_FORMAT_R16G16B16A16_SINT:         return Format::RGBA16SInt;
        case DXGI_FORMAT_R16G16B16A16_FLOAT:        return Format::RGBA16Float;

        case DXGI_FORMAT_R32G32B32A32_UINT:         return Format::RGBA32UInt;
        case DXGI_FORMAT_R32G32B32A32_SINT:         return Format::RGBA32SInt;
        case DXGI_FORMAT_R32G32B32A32_FLOAT:        return Format::RGBA32Float;

        /* --- BGRA color formats --- */
        case DXGI_FORMAT_B8G8R8A8_UNORM:            return Format::BGRA8UNorm;
        case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:       return Format::BGRA8UNorm_sRGB;

        /* --- Depth-stencil formats --- */
        case DXGI_FORMAT_R16_TYPELESS:              /* pass */
        case DXGI_FORMAT_D16_UNORM:                 return Format::D16UNorm;
        case DXGI_FORMAT_R32_TYPELESS:              /* pass */
        case DXGI_FORMAT_D32_FLOAT:                 return Format::D32Float;
        case DXGI_FORMAT_R24G8_TYPELESS:            /* pass */
        case DXGI_FORMAT_D24_UNORM_S8_UINT:         return Format::D24UNormS8UInt;
        case DXGI_FORMAT_R32G8X24_TYPELESS:         /* pass */
        case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:      return Format::D32FloatS8X24UInt;

        /* --- Compressed color formats --- */
        case DXGI_FORMAT_BC1_UNORM:                 return Format::BC1RGBA;
        case DXGI_FORMAT_BC2_UNORM:                 return Format::BC2RGBA;
        case DXGI_FORMAT_BC3_UNORM:                 return Format::BC3RGBA;

        default:                                    return Format::Undefined;
    }
}

StorageBufferType Unmap(const D3D_SHADER_INPUT_TYPE inputType)
{
    switch (inputType)
    {
        case D3D_SIT_UAV_RWTYPED:                   return StorageBufferType::RWBuffer;
        case D3D_SIT_STRUCTURED:                    return StorageBufferType::StructuredBuffer;
        case D3D_SIT_UAV_RWSTRUCTURED:              return StorageBufferType::RWStructuredBuffer;
        case D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER: return StorageBufferType::RWStructuredBuffer;
        case D3D_SIT_BYTEADDRESS:                   return StorageBufferType::ByteAddressBuffer;
        case D3D_SIT_UAV_RWBYTEADDRESS:             return StorageBufferType::RWByteAddressBuffer;
        case D3D_SIT_UAV_APPEND_STRUCTURED:         return StorageBufferType::AppendStructuredBuffer;
        case D3D_SIT_UAV_CONSUME_STRUCTURED:        return StorageBufferType::ConsumeStructuredBuffer;
        default:                                    return StorageBufferType::Undefined;
    }
}

SystemValue Unmap(const D3D_NAME name)
{
    switch (name)
    {
        case D3D_NAME_CLIP_DISTANCE:                return SystemValue::ClipDistance;
        case D3D_NAME_TARGET:                       return SystemValue::Color;
        case D3D_NAME_CULL_DISTANCE:                return SystemValue::CullDistance;
        case D3D_NAME_DEPTH:                        return SystemValue::Depth;
        case D3D_NAME_DEPTH_GREATER_EQUAL:          return SystemValue::DepthGreater;
        case D3D_NAME_DEPTH_LESS_EQUAL:             return SystemValue::DepthLess;
        case D3D_NAME_IS_FRONT_FACE:                return SystemValue::FrontFacing;
        case D3D_NAME_INSTANCE_ID:                  return SystemValue::InstanceID;
        case D3D_NAME_POSITION:                     return SystemValue::Position;
        case D3D_NAME_PRIMITIVE_ID:                 return SystemValue::PrimitiveID;
        case D3D_NAME_RENDER_TARGET_ARRAY_INDEX:    return SystemValue::RenderTargetIndex;
        case D3D_NAME_COVERAGE:                     return SystemValue::SampleMask;
        case D3D_NAME_SAMPLE_INDEX:                 return SystemValue::SampleID;
        #if defined LLGL_DX_ENABLE_D3D12 || LLGL_D3D11_ENABLE_FEATURELEVEL >= 3
        case D3D_NAME_STENCIL_REF:                  return SystemValue::Stencil;
        #endif
        case D3D_NAME_VERTEX_ID:                    return SystemValue::VertexID;
        case D3D_NAME_VIEWPORT_ARRAY_INDEX:         return SystemValue::ViewportIndex;
        default:                                    return SystemValue::Undefined;
    }
}

ResourceType Unmap(const D3D_SRV_DIMENSION dimension)
{
    switch (dimension)
    {
        case D3D_SRV_DIMENSION_BUFFER:
        case D3D_SRV_DIMENSION_BUFFEREX:
            return ResourceType::Buffer;
        case D3D_SRV_DIMENSION_TEXTURE1D:
        case D3D_SRV_DIMENSION_TEXTURE1DARRAY:
        case D3D_SRV_DIMENSION_TEXTURE2D:
        case D3D_SRV_DIMENSION_TEXTURE2DARRAY:
        case D3D_SRV_DIMENSION_TEXTURE2DMS:
        case D3D_SRV_DIMENSION_TEXTURE2DMSARRAY:
        case D3D_SRV_DIMENSION_TEXTURE3D:
        case D3D_SRV_DIMENSION_TEXTURECUBE:
        case D3D_SRV_DIMENSION_TEXTURECUBEARRAY:
            return ResourceType::Texture;
        default:
            return ResourceType::Undefined;
    }
}

DXGI_FORMAT ToDXGIFormatDSV(const DXGI_FORMAT format)
{
    switch (format)
    {
        case DXGI_FORMAT_R16_TYPELESS:      return DXGI_FORMAT_D16_UNORM;
        case DXGI_FORMAT_R32_TYPELESS:      return DXGI_FORMAT_D32_FLOAT;
        case DXGI_FORMAT_R24G8_TYPELESS:    return DXGI_FORMAT_D24_UNORM_S8_UINT;
        case DXGI_FORMAT_R32G8X24_TYPELESS: return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
        default:                            return format;
    }
}

DXGI_FORMAT ToDXGIFormatSRV(const DXGI_FORMAT format)
{
    switch (format)
    {
        case DXGI_FORMAT_R16_TYPELESS:      return DXGI_FORMAT_R16_UNORM;
        case DXGI_FORMAT_R32_TYPELESS:      return DXGI_FORMAT_R32_FLOAT;
        case DXGI_FORMAT_R24G8_TYPELESS:    return DXGI_FORMAT_UNKNOWN;
        case DXGI_FORMAT_R32G8X24_TYPELESS: return DXGI_FORMAT_UNKNOWN;
        default:                            return format;
    }
}

bool HasStencilComponent(const DXGI_FORMAT format)
{
    return (format == DXGI_FORMAT_D24_UNORM_S8_UINT || format == DXGI_FORMAT_D32_FLOAT_S8X24_UINT);
}

bool IsDXGIFormatSRGB(const DXGI_FORMAT format)
{
    switch (format)
    {
        case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
        case DXGI_FORMAT_BC1_UNORM_SRGB:
        case DXGI_FORMAT_BC2_UNORM_SRGB:
        case DXGI_FORMAT_BC3_UNORM_SRGB:
        case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
        case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
        case DXGI_FORMAT_BC7_UNORM_SRGB:
            return true;
        default:
            return false;
    }
}


} // /namespace DXTypes

} // /namespace LLGL



// ================================================================================
