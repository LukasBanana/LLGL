/*
 * D3D11Types.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11Types.h"
#include "../DXCommon/DXTypes.h"
#include <stdexcept>
#include <string>


namespace LLGL
{

namespace D3D11Types
{


DXGI_FORMAT Map(const VectorType vectorType)
{
    return DXTypes::Map(vectorType);
}

DXGI_FORMAT Map(const DataType dataType)
{
    return DXTypes::Map(dataType);
}

DXGI_FORMAT Map(const TextureFormat textureFormat)
{
    return DXTypes::Map(textureFormat);
}

D3D_PRIMITIVE_TOPOLOGY Map(const PrimitiveTopology topology)
{
    return DXTypes::Map(topology);
}

D3D11_FILL_MODE Map(const PolygonMode polygonMode)
{
    switch (polygonMode)
    {
        case PolygonMode::Fill:         return D3D11_FILL_SOLID;
        case PolygonMode::Wireframe:    return D3D11_FILL_WIREFRAME;
        case PolygonMode::Points:       break;
    }
    DXTypes::MapFailed("PolygonMode", "D3D11_FILL_MODE");
}

D3D11_CULL_MODE Map(const CullMode cullMode)
{
    switch (cullMode)
    {
        case CullMode::Disabled:    return D3D11_CULL_NONE;
        case CullMode::Front:       return D3D11_CULL_FRONT;
        case CullMode::Back:        return D3D11_CULL_BACK;
    }
    DXTypes::MapFailed("CullMode", "D3D11_CULL_MODE");
}

// see https://msdn.microsoft.com/en-us/library/windows/desktop/ff476086(v=vs.85).aspx
D3D11_BLEND Map(const BlendOp blendOp)
{
    switch (blendOp)
    {
        case BlendOp::Zero:             return D3D11_BLEND_ZERO;
        case BlendOp::One:              return D3D11_BLEND_ONE;
        case BlendOp::SrcColor:         return D3D11_BLEND_SRC_COLOR;
        case BlendOp::InvSrcColor:      return D3D11_BLEND_INV_SRC_COLOR;
        case BlendOp::SrcAlpha:         return D3D11_BLEND_SRC_ALPHA;
        case BlendOp::InvSrcAlpha:      return D3D11_BLEND_INV_SRC_ALPHA;
        case BlendOp::DstColor:         return D3D11_BLEND_DEST_COLOR;
        case BlendOp::InvDstColor:      return D3D11_BLEND_INV_DEST_COLOR;
        case BlendOp::DstAlpha:         return D3D11_BLEND_DEST_ALPHA;
        case BlendOp::InvDstAlpha:      return D3D11_BLEND_INV_DEST_ALPHA;
        case BlendOp::SrcAlphaSaturate: return D3D11_BLEND_SRC_ALPHA_SAT;
        case BlendOp::BlendFactor:      return D3D11_BLEND_BLEND_FACTOR;
        case BlendOp::InvBlendFactor:   return D3D11_BLEND_INV_BLEND_FACTOR;
        case BlendOp::Src1Color:        return D3D11_BLEND_SRC1_COLOR;
        case BlendOp::InvSrc1Color:     return D3D11_BLEND_INV_SRC1_COLOR;
        case BlendOp::Src1Alpha:        return D3D11_BLEND_SRC1_ALPHA;
        case BlendOp::InvSrc1Alpha:     return D3D11_BLEND_INV_SRC1_ALPHA;
    }
    DXTypes::MapFailed("BlendOp", "D3D11_BLEND");
}

D3D11_BLEND_OP Map(const BlendArithmetic blendArithmetic)
{
    switch (blendArithmetic)
    {
        case BlendArithmetic::Add:          return D3D11_BLEND_OP_ADD;
        case BlendArithmetic::Subtract:     return D3D11_BLEND_OP_SUBTRACT;
        case BlendArithmetic::RevSubtract:  return D3D11_BLEND_OP_REV_SUBTRACT;
        case BlendArithmetic::Min:          return D3D11_BLEND_OP_MIN;
        case BlendArithmetic::Max:          return D3D11_BLEND_OP_MAX;
    }
    DXTypes::MapFailed("BlendArithmetic", "D3D11_BLEND_OP");
}

D3D11_COMPARISON_FUNC Map(const CompareOp compareOp)
{
    switch (compareOp)
    {
        case CompareOp::Never:          return D3D11_COMPARISON_NEVER;
        case CompareOp::Less:           return D3D11_COMPARISON_LESS;
        case CompareOp::Equal:          return D3D11_COMPARISON_EQUAL;
        case CompareOp::LessEqual:      return D3D11_COMPARISON_LESS_EQUAL;
        case CompareOp::Greater:        return D3D11_COMPARISON_GREATER;
        case CompareOp::NotEqual:       return D3D11_COMPARISON_NOT_EQUAL;
        case CompareOp::GreaterEqual:   return D3D11_COMPARISON_GREATER_EQUAL;
        case CompareOp::Ever:           return D3D11_COMPARISON_ALWAYS;
    }
    DXTypes::MapFailed("CompareOp", "D3D11_COMPARISON_FUNC");
}

D3D11_STENCIL_OP Map(const StencilOp stencilOp)
{
    switch (stencilOp)
    {
        case StencilOp::Keep:       return D3D11_STENCIL_OP_KEEP;
        case StencilOp::Zero:       return D3D11_STENCIL_OP_ZERO;
        case StencilOp::Replace:    return D3D11_STENCIL_OP_REPLACE;
        case StencilOp::IncClamp:   return D3D11_STENCIL_OP_INCR_SAT;
        case StencilOp::DecClamp:   return D3D11_STENCIL_OP_DECR_SAT;
        case StencilOp::Invert:     return D3D11_STENCIL_OP_INVERT;
        case StencilOp::IncWrap:    return D3D11_STENCIL_OP_INCR;
        case StencilOp::DecWrap:    return D3D11_STENCIL_OP_DECR;
    }
    DXTypes::MapFailed("StencilOp", "D3D11_STENCIL_OP");
}

D3D11_FILTER Map(const SamplerDescriptor& samplerDesc)
{
    if (samplerDesc.maxAnisotropy > 1)
        return D3D11_FILTER_ANISOTROPIC;

    switch (samplerDesc.minFilter)
    {
        case TextureFilter::Nearest:
        {
            switch (samplerDesc.magFilter)
            {
                case TextureFilter::Nearest:
                {
                    switch (samplerDesc.mipMapFilter)
                    {
                        case TextureFilter::Nearest:    return D3D11_FILTER_MIN_MAG_MIP_POINT;
                        case TextureFilter::Linear:     return D3D11_FILTER_MIN_MAG_POINT_MIP_LINEAR;
                    }
                }
                break;

                case TextureFilter::Linear:
                {
                    switch (samplerDesc.mipMapFilter)
                    {
                        case TextureFilter::Nearest:    return D3D11_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
                        case TextureFilter::Linear:     return D3D11_FILTER_MIN_POINT_MAG_MIP_LINEAR;
                    }
                }
                break;
            }
        }
        break;

        case TextureFilter::Linear:
        {
            switch (samplerDesc.magFilter)
            {
                case TextureFilter::Nearest:
                {
                    switch (samplerDesc.mipMapFilter)
                    {
                        case TextureFilter::Nearest:    return D3D11_FILTER_MIN_LINEAR_MAG_MIP_POINT;
                        case TextureFilter::Linear:     return D3D11_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
                    }
                }
                break;

                case TextureFilter::Linear:
                {
                    switch (samplerDesc.mipMapFilter)
                    {
                        case TextureFilter::Nearest:    return D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT;
                        case TextureFilter::Linear:     return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
                    }
                }
                break;
            }
        }
        break;
    }

    DXTypes::MapFailed("SamplerDescriptor", "D3D11_FILTER");
}

D3D11_TEXTURE_ADDRESS_MODE Map(const TextureWrap textureWrap)
{
    switch (textureWrap)
    {
        case TextureWrap::Repeat:       return D3D11_TEXTURE_ADDRESS_WRAP;
        case TextureWrap::Mirror:       return D3D11_TEXTURE_ADDRESS_MIRROR;
        case TextureWrap::Clamp:        return D3D11_TEXTURE_ADDRESS_CLAMP;
        case TextureWrap::Border:       return D3D11_TEXTURE_ADDRESS_BORDER;
        case TextureWrap::MirrorOnce:   return D3D11_TEXTURE_ADDRESS_MIRROR_ONCE;
    }
    DXTypes::MapFailed("TextureWrap", "D3D11_TEXTURE_ADDRESS_MODE");
}

D3D11_QUERY Map(const QueryDescriptor& queryDesc)
{
    if (queryDesc.renderCondition)
    {
        switch (queryDesc.type)
        {
            case QueryType::SamplesPassed:                      /* pass */
            case QueryType::AnySamplesPassed:                   /* pass */
            case QueryType::AnySamplesPassedConservative:       return D3D11_QUERY_OCCLUSION_PREDICATE;
            case QueryType::StreamOutOverflow:                  return D3D11_QUERY_SO_OVERFLOW_PREDICATE;
            default:                                            break;
        }
    }
    else
    {
        switch (queryDesc.type)
        {
            case QueryType::SamplesPassed:                      /* pass */
            case QueryType::AnySamplesPassed:                   /* pass */
            case QueryType::AnySamplesPassedConservative:       return D3D11_QUERY_OCCLUSION;
            case QueryType::TimeElapsed:                        return D3D11_QUERY_TIMESTAMP_DISJOINT;
            case QueryType::StreamOutOverflow:                  break;
            case QueryType::StreamOutPrimitivesWritten:         return D3D11_QUERY_SO_STATISTICS;
            case QueryType::PipelineStatistics:                 return D3D11_QUERY_PIPELINE_STATISTICS;
        }
    }
    DXTypes::MapFailed("QueryType", "D3D11_QUERY");
}

D3D11_MAP Map(const BufferCPUAccess cpuAccess)
{
    switch (cpuAccess)
    {
        case BufferCPUAccess::ReadOnly:     return D3D11_MAP_READ;
        case BufferCPUAccess::WriteOnly:    return D3D11_MAP_WRITE;
        case BufferCPUAccess::ReadWrite:    return D3D11_MAP_READ_WRITE;
                                          /*return D3D11_MAP_WRITE_DISCARD;
                                            return D3D11_MAP_WRITE_NO_OVERWRITE;*/
    }
    DXTypes::MapFailed("BufferCPUAccess", "D3D11_MAP");
}

D3D11_SRV_DIMENSION Map(const TextureType textureType)
{
    switch (textureType)
    {
        case TextureType::Texture1D:        return D3D11_SRV_DIMENSION_TEXTURE1D;
        case TextureType::Texture2D:        return D3D11_SRV_DIMENSION_TEXTURE2D;
        case TextureType::Texture3D:        return D3D11_SRV_DIMENSION_TEXTURE3D;
        case TextureType::TextureCube:      return D3D11_SRV_DIMENSION_TEXTURECUBE;
        case TextureType::Texture1DArray:   return D3D11_SRV_DIMENSION_TEXTURE1DARRAY;
        case TextureType::Texture2DArray:   return D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
        case TextureType::TextureCubeArray: return D3D11_SRV_DIMENSION_TEXTURECUBEARRAY;
        case TextureType::Texture2DMS:      return D3D11_SRV_DIMENSION_TEXTURE2DMS;
        case TextureType::Texture2DMSArray: return D3D11_SRV_DIMENSION_TEXTURE2DMSARRAY;
    }
    DXTypes::MapFailed("TextureType", "D3D11_SRV_DIMENSION");
}

TextureFormat Unmap(const DXGI_FORMAT format)
{
    return DXTypes::Unmap(format);
}


} // /namespace D3D11Types

} // /namespace LLGL



// ================================================================================
