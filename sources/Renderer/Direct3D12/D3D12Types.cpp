/*
 * D3D12Types.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D12Types.h"
#include <stdexcept>
#include <string>


namespace LLGL
{

namespace D3D12Types
{


D3D12_FILL_MODE Map(const PolygonMode polygonMode)
{
    switch (polygonMode)
    {
        case PolygonMode::Fill:         return D3D12_FILL_MODE_SOLID;
        case PolygonMode::Wireframe:    return D3D12_FILL_MODE_WIREFRAME;
        case PolygonMode::Points:       break;
    }
    LLGL_TRAP_DX_MAP(PolygonMode, polygonMode, D3D12_FILL_MODE);
}

D3D12_CULL_MODE Map(const CullMode cullMode)
{
    switch (cullMode)
    {
        case CullMode::Disabled:    return D3D12_CULL_MODE_NONE;
        case CullMode::Front:       return D3D12_CULL_MODE_FRONT;
        case CullMode::Back:        return D3D12_CULL_MODE_BACK;
    }
    LLGL_TRAP_DX_MAP(CullMode, cullMode, D3D12_CULL_MODE);
}

// see https://msdn.microsoft.com/en-us/library/windows/desktop/dn770338(v=vs.85).aspx
D3D12_BLEND Map(const BlendOp blendOp)
{
    switch (blendOp)
    {
        case BlendOp::Zero:             return D3D12_BLEND_ZERO;
        case BlendOp::One:              return D3D12_BLEND_ONE;
        case BlendOp::SrcColor:         return D3D12_BLEND_SRC_COLOR;
        case BlendOp::InvSrcColor:      return D3D12_BLEND_INV_SRC_COLOR;
        case BlendOp::SrcAlpha:         return D3D12_BLEND_SRC_ALPHA;
        case BlendOp::InvSrcAlpha:      return D3D12_BLEND_INV_SRC_ALPHA;
        case BlendOp::DstColor:         return D3D12_BLEND_DEST_COLOR;
        case BlendOp::InvDstColor:      return D3D12_BLEND_INV_DEST_COLOR;
        case BlendOp::DstAlpha:         return D3D12_BLEND_DEST_ALPHA;
        case BlendOp::InvDstAlpha:      return D3D12_BLEND_INV_DEST_ALPHA;
        case BlendOp::SrcAlphaSaturate: return D3D12_BLEND_SRC_ALPHA_SAT;
        case BlendOp::BlendFactor:      return D3D12_BLEND_BLEND_FACTOR;
        case BlendOp::InvBlendFactor:   return D3D12_BLEND_INV_BLEND_FACTOR;
        case BlendOp::Src1Color:        return D3D12_BLEND_SRC1_COLOR;
        case BlendOp::InvSrc1Color:     return D3D12_BLEND_INV_SRC1_COLOR;
        case BlendOp::Src1Alpha:        return D3D12_BLEND_SRC1_ALPHA;
        case BlendOp::InvSrc1Alpha:     return D3D12_BLEND_INV_SRC1_ALPHA;
    }
    LLGL_TRAP_DX_MAP(BlendOp, blendOp, D3D12_BLEND);
}

D3D12_BLEND_OP Map(const BlendArithmetic blendArithmetic)
{
    switch (blendArithmetic)
    {
        case BlendArithmetic::Add:          return D3D12_BLEND_OP_ADD;
        case BlendArithmetic::Subtract:     return D3D12_BLEND_OP_SUBTRACT;
        case BlendArithmetic::RevSubtract:  return D3D12_BLEND_OP_REV_SUBTRACT;
        case BlendArithmetic::Min:          return D3D12_BLEND_OP_MIN;
        case BlendArithmetic::Max:          return D3D12_BLEND_OP_MAX;
    }
    LLGL_TRAP_DX_MAP(BlendArithmetic, blendArithmetic, D3D12_BLEND_OP);
}

D3D12_COMPARISON_FUNC Map(const CompareOp compareOp)
{
    switch (compareOp)
    {
        case CompareOp::NeverPass:      return D3D12_COMPARISON_FUNC_NEVER;
        case CompareOp::Less:           return D3D12_COMPARISON_FUNC_LESS;
        case CompareOp::Equal:          return D3D12_COMPARISON_FUNC_EQUAL;
        case CompareOp::LessEqual:      return D3D12_COMPARISON_FUNC_LESS_EQUAL;
        case CompareOp::Greater:        return D3D12_COMPARISON_FUNC_GREATER;
        case CompareOp::NotEqual:       return D3D12_COMPARISON_FUNC_NOT_EQUAL;
        case CompareOp::GreaterEqual:   return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
        case CompareOp::AlwaysPass:     return D3D12_COMPARISON_FUNC_ALWAYS;
    }
    LLGL_TRAP_DX_MAP(CompareOp, compareOp, D3D12_COMPARISON_FUNC);
}

D3D12_STENCIL_OP Map(const StencilOp stencilOp)
{
    switch (stencilOp)
    {
        case StencilOp::Keep:       return D3D12_STENCIL_OP_KEEP;
        case StencilOp::Zero:       return D3D12_STENCIL_OP_ZERO;
        case StencilOp::Replace:    return D3D12_STENCIL_OP_REPLACE;
        case StencilOp::IncClamp:   return D3D12_STENCIL_OP_INCR_SAT;
        case StencilOp::DecClamp:   return D3D12_STENCIL_OP_DECR_SAT;
        case StencilOp::Invert:     return D3D12_STENCIL_OP_INVERT;
        case StencilOp::IncWrap:    return D3D12_STENCIL_OP_INCR;
        case StencilOp::DecWrap:    return D3D12_STENCIL_OP_DECR;
    }
    LLGL_TRAP_DX_MAP(StencilOp, stencilOp, D3D12_STENCIL_OP);
}

D3D12_FILTER Map(const SamplerDescriptor& samplerDesc)
{
    if (samplerDesc.compareEnabled)
    {
        if (samplerDesc.maxAnisotropy > 1)
            return D3D12_FILTER_COMPARISON_ANISOTROPIC;
        else if (samplerDesc.minFilter == SamplerFilter::Nearest)
        {
            if (samplerDesc.magFilter == SamplerFilter::Nearest)
            {
                if (samplerDesc.mipMapFilter == SamplerFilter::Nearest) return D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT;
                if (samplerDesc.mipMapFilter == SamplerFilter::Linear ) return D3D12_FILTER_COMPARISON_MIN_MAG_POINT_MIP_LINEAR;
            }
            else if (samplerDesc.magFilter == SamplerFilter::Linear)
            {
                if (samplerDesc.mipMapFilter == SamplerFilter::Nearest) return D3D12_FILTER_COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT;
                if (samplerDesc.mipMapFilter == SamplerFilter::Linear ) return D3D12_FILTER_COMPARISON_MIN_POINT_MAG_MIP_LINEAR;
            }
        }
        else if (samplerDesc.minFilter == SamplerFilter::Linear)
        {
            if (samplerDesc.magFilter == SamplerFilter::Nearest)
            {
                if (samplerDesc.mipMapFilter == SamplerFilter::Nearest) return D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_MIP_POINT;
                if (samplerDesc.mipMapFilter == SamplerFilter::Linear ) return D3D12_FILTER_COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
            }
            else if (samplerDesc.magFilter == SamplerFilter::Linear)
            {
                if (samplerDesc.mipMapFilter == SamplerFilter::Nearest) return D3D12_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
                if (samplerDesc.mipMapFilter == SamplerFilter::Linear ) return D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
            }
        }
    }
    else
    {
        if (samplerDesc.maxAnisotropy > 1)
            return D3D12_FILTER_ANISOTROPIC;
        else if (samplerDesc.minFilter == SamplerFilter::Nearest)
        {
            if (samplerDesc.magFilter == SamplerFilter::Nearest)
            {
                if (samplerDesc.mipMapFilter == SamplerFilter::Nearest) return D3D12_FILTER_MIN_MAG_MIP_POINT;
                if (samplerDesc.mipMapFilter == SamplerFilter::Linear ) return D3D12_FILTER_MIN_MAG_POINT_MIP_LINEAR;
            }
            else if (samplerDesc.magFilter == SamplerFilter::Linear)
            {
                if (samplerDesc.mipMapFilter == SamplerFilter::Nearest) return D3D12_FILTER_MIN_POINT_MAG_LINEAR_MIP_POINT;
                if (samplerDesc.mipMapFilter == SamplerFilter::Linear ) return D3D12_FILTER_MIN_POINT_MAG_MIP_LINEAR;
            }
        }
        else if (samplerDesc.minFilter == SamplerFilter::Linear)
        {
            if (samplerDesc.magFilter == SamplerFilter::Nearest)
            {
                if (samplerDesc.mipMapFilter == SamplerFilter::Nearest) return D3D12_FILTER_MIN_LINEAR_MAG_MIP_POINT;
                if (samplerDesc.mipMapFilter == SamplerFilter::Linear ) return D3D12_FILTER_MIN_LINEAR_MAG_POINT_MIP_LINEAR;
            }
            else if (samplerDesc.magFilter == SamplerFilter::Linear)
            {
                if (samplerDesc.mipMapFilter == SamplerFilter::Nearest) return D3D12_FILTER_MIN_MAG_LINEAR_MIP_POINT;
                if (samplerDesc.mipMapFilter == SamplerFilter::Linear ) return D3D12_FILTER_MIN_MAG_MIP_LINEAR;
            }
        }
    }
    LLGL_TRAP_DX_MAP_NOVALUE(SamplerDescriptor, D3D12_FILTER);
}

D3D12_TEXTURE_ADDRESS_MODE Map(const SamplerAddressMode addressMode)
{
    switch (addressMode)
    {
        case SamplerAddressMode::Repeat:       return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        case SamplerAddressMode::Mirror:       return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
        case SamplerAddressMode::Clamp:        return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        case SamplerAddressMode::Border:       return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        case SamplerAddressMode::MirrorOnce:   return D3D12_TEXTURE_ADDRESS_MODE_MIRROR_ONCE;
    }
    LLGL_TRAP_DX_MAP(SamplerAddressMode, addressMode, D3D12_TEXTURE_ADDRESS_MODE);
}

D3D12_LOGIC_OP Map(const LogicOp logicOp)
{
    switch (logicOp)
    {
        case LogicOp::Disabled:     return D3D12_LOGIC_OP_NOOP; // use default value when it's disabled
        case LogicOp::Clear:        return D3D12_LOGIC_OP_CLEAR;
        case LogicOp::Set:          return D3D12_LOGIC_OP_SET;
        case LogicOp::Copy:         return D3D12_LOGIC_OP_COPY;
        case LogicOp::CopyInverted: return D3D12_LOGIC_OP_COPY_INVERTED;
        case LogicOp::NoOp:         return D3D12_LOGIC_OP_NOOP;
        case LogicOp::Invert:       return D3D12_LOGIC_OP_INVERT;
        case LogicOp::AND:          return D3D12_LOGIC_OP_AND;
        case LogicOp::ANDReverse:   return D3D12_LOGIC_OP_AND_REVERSE;
        case LogicOp::ANDInverted:  return D3D12_LOGIC_OP_AND_INVERTED;
        case LogicOp::NAND:         return D3D12_LOGIC_OP_NAND;
        case LogicOp::OR:           return D3D12_LOGIC_OP_OR;
        case LogicOp::ORReverse:    return D3D12_LOGIC_OP_OR_REVERSE;
        case LogicOp::ORInverted:   return D3D12_LOGIC_OP_OR_INVERTED;
        case LogicOp::NOR:          return D3D12_LOGIC_OP_NOR;
        case LogicOp::XOR:          return D3D12_LOGIC_OP_XOR;
        case LogicOp::Equiv:        return D3D12_LOGIC_OP_EQUIV;
    }
    LLGL_TRAP_DX_MAP(LogicOp, logicOp, D3D12_LOGIC_OP);
}

D3D12_SHADER_COMPONENT_MAPPING Map(const TextureSwizzle textureSwizzle)
{
    switch (textureSwizzle)
    {
        case TextureSwizzle::Zero:  return D3D12_SHADER_COMPONENT_MAPPING_FORCE_VALUE_0;
        case TextureSwizzle::One:   return D3D12_SHADER_COMPONENT_MAPPING_FORCE_VALUE_1;
        case TextureSwizzle::Red:   return D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_0;
        case TextureSwizzle::Green: return D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_1;
        case TextureSwizzle::Blue:  return D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_2;
        case TextureSwizzle::Alpha: return D3D12_SHADER_COMPONENT_MAPPING_FROM_MEMORY_COMPONENT_3;
    }
    LLGL_TRAP_DX_MAP(TextureSwizzle, textureSwizzle, D3D12_SHADER_COMPONENT_MAPPING);
}

UINT Map(const TextureSwizzleRGBA& textureSwizzle)
{
    return D3D12_ENCODE_SHADER_4_COMPONENT_MAPPING(
        Map(textureSwizzle.r),
        Map(textureSwizzle.g),
        Map(textureSwizzle.b),
        Map(textureSwizzle.a)
    );
}

D3D12_SRV_DIMENSION MapSrvDimension(const TextureType textureType)
{
    switch (textureType)
    {
        case TextureType::Texture1D:        return D3D12_SRV_DIMENSION_TEXTURE1D;
        case TextureType::Texture2D:        return D3D12_SRV_DIMENSION_TEXTURE2D;
        case TextureType::Texture3D:        return D3D12_SRV_DIMENSION_TEXTURE3D;
        case TextureType::TextureCube:      return D3D12_SRV_DIMENSION_TEXTURECUBE;
        case TextureType::Texture1DArray:   return D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
        case TextureType::Texture2DArray:   return D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
        case TextureType::TextureCubeArray: return D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
        case TextureType::Texture2DMS:      return D3D12_SRV_DIMENSION_TEXTURE2DMS;
        case TextureType::Texture2DMSArray: return D3D12_SRV_DIMENSION_TEXTURE2DMSARRAY;
    }
    LLGL_TRAP_DX_MAP(TextureType, textureType, D3D12_SRV_DIMENSION);
}

D3D12_UAV_DIMENSION MapUavDimension(const TextureType textureType)
{
    switch (textureType)
    {
        case TextureType::Texture1D:        return D3D12_UAV_DIMENSION_TEXTURE1D;
        case TextureType::Texture2D:        return D3D12_UAV_DIMENSION_TEXTURE2D;
        case TextureType::Texture3D:        return D3D12_UAV_DIMENSION_TEXTURE3D;
        case TextureType::TextureCube:      return D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
        case TextureType::Texture1DArray:   return D3D12_UAV_DIMENSION_TEXTURE1DARRAY;
        case TextureType::Texture2DArray:   return D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
        case TextureType::TextureCubeArray: return D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
        case TextureType::Texture2DMS:      break;
        case TextureType::Texture2DMSArray: break;
    }
    LLGL_TRAP_DX_MAP(TextureType, textureType, D3D12_UAV_DIMENSION);
}

D3D12_RESOURCE_DIMENSION MapResourceDimension(const TextureType textureType)
{
    switch (textureType)
    {
        case TextureType::Texture1D:        /* pass */
        case TextureType::Texture1DArray:   return D3D12_RESOURCE_DIMENSION_TEXTURE1D;
        case TextureType::Texture2D:        /* pass */
        case TextureType::Texture2DArray:   /* pass */
        case TextureType::TextureCube:      /* pass */
        case TextureType::TextureCubeArray: /* pass */
        case TextureType::Texture2DMS:      /* pass */
        case TextureType::Texture2DMSArray: return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        case TextureType::Texture3D:        return D3D12_RESOURCE_DIMENSION_TEXTURE3D;
    }
    LLGL_TRAP_DX_MAP(TextureType, textureType, D3D12_RESOURCE_DIMENSION);
}

D3D12_QUERY_TYPE MapQueryType(const QueryType queryType)
{
    switch (queryType)
    {
        case QueryType::SamplesPassed:                  return D3D12_QUERY_TYPE_OCCLUSION;
        case QueryType::AnySamplesPassed:               /* pass */
        case QueryType::AnySamplesPassedConservative:   return D3D12_QUERY_TYPE_BINARY_OCCLUSION;
        case QueryType::TimeElapsed:                    return D3D12_QUERY_TYPE_TIMESTAMP;
        case QueryType::StreamOutPrimitivesWritten:     return D3D12_QUERY_TYPE_SO_STATISTICS_STREAM0;
        case QueryType::StreamOutOverflow:              return D3D12_QUERY_TYPE_SO_STATISTICS_STREAM0; // Same query type, different interpretation
        case QueryType::PipelineStatistics:             return D3D12_QUERY_TYPE_PIPELINE_STATISTICS;
    }
    LLGL_TRAP_DX_MAP(QueryType, queryType, D3D12_QUERY_TYPE);
}

D3D12_QUERY_HEAP_TYPE MapQueryHeapType(const QueryType queryType)
{
    switch (queryType)
    {
        case QueryType::SamplesPassed:                  /* pass */
        case QueryType::AnySamplesPassed:               /* pass */
        case QueryType::AnySamplesPassedConservative:   return D3D12_QUERY_HEAP_TYPE_OCCLUSION;
        case QueryType::TimeElapsed:                    return D3D12_QUERY_HEAP_TYPE_TIMESTAMP;
        case QueryType::StreamOutPrimitivesWritten:     /* pass */
        case QueryType::StreamOutOverflow:              return D3D12_QUERY_HEAP_TYPE_SO_STATISTICS;
        case QueryType::PipelineStatistics:             return D3D12_QUERY_HEAP_TYPE_PIPELINE_STATISTICS;
    }
    LLGL_TRAP_DX_MAP(QueryType, queryType, D3D12_QUERY_HEAP_TYPE);
}

Format Unmap(const DXGI_FORMAT format)
{
    return DXTypes::Unmap(format);
}


} // /namespace D3D12Types

} // /namespace LLGL



// ================================================================================
