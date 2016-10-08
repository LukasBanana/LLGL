/*
 * D3D12Types.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12Types.h"
#include "../DXCommon/DXTypes.h"
#include <stdexcept>
#include <string>


namespace LLGL
{

namespace D3D12Types
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

D3D12_FILL_MODE Map(const PolygonMode polygonMode)
{
    switch (polygonMode)
    {
        case PolygonMode::Fill:         return D3D12_FILL_MODE_SOLID;
        case PolygonMode::Wireframe:    return D3D12_FILL_MODE_WIREFRAME;
        case PolygonMode::Points:       break;
    }
    DXTypes::MapFailed("PolygonMode", "D3D12_FILL_MODE");
}

D3D12_CULL_MODE Map(const CullMode cullMode)
{
    switch (cullMode)
    {
        case CullMode::Disabled:    return D3D12_CULL_MODE_NONE;
        case CullMode::Front:       return D3D12_CULL_MODE_FRONT;
        case CullMode::Back:        return D3D12_CULL_MODE_BACK;
    }
    DXTypes::MapFailed("CullMode", "D3D12_CULL_MODE");
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
        case BlendOp::DestColor:        return D3D12_BLEND_DEST_COLOR;
        case BlendOp::InvDestColor:     return D3D12_BLEND_INV_DEST_COLOR;
        case BlendOp::DestAlpha:        return D3D12_BLEND_DEST_ALPHA;
        case BlendOp::InvDestAlpha:     return D3D12_BLEND_INV_DEST_ALPHA;
        case BlendOp::SrcAlphaSaturate: return D3D12_BLEND_SRC_ALPHA_SAT;
        case BlendOp::BlendFactor:      return D3D12_BLEND_BLEND_FACTOR;
        case BlendOp::InvBlendFactor:   return D3D12_BLEND_INV_BLEND_FACTOR;
        case BlendOp::Src1Color:        return D3D12_BLEND_SRC1_COLOR;
        case BlendOp::InvSrc1Color:     return D3D12_BLEND_INV_SRC1_COLOR;
        case BlendOp::Src1Alpha:        return D3D12_BLEND_SRC1_ALPHA;
        case BlendOp::InvSrc1Alpha:     return D3D12_BLEND_INV_SRC1_ALPHA;
    }
    DXTypes::MapFailed("BlendOp", "D3D12_BLEND");
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
    DXTypes::MapFailed("BlendArithmetic", "D3D12_BLEND_OP");
}

D3D12_COMPARISON_FUNC Map(const CompareOp compareOp)
{
    switch (compareOp)
    {
        case CompareOp::Never:          return D3D12_COMPARISON_FUNC_NEVER;
        case CompareOp::Less:           return D3D12_COMPARISON_FUNC_LESS;
        case CompareOp::Equal:          return D3D12_COMPARISON_FUNC_EQUAL;
        case CompareOp::LessEqual:      return D3D12_COMPARISON_FUNC_LESS_EQUAL;
        case CompareOp::Greater:        return D3D12_COMPARISON_FUNC_GREATER;
        case CompareOp::NotEqual:       return D3D12_COMPARISON_FUNC_NOT_EQUAL;
        case CompareOp::GreaterEqual:   return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
        case CompareOp::Ever:           return D3D12_COMPARISON_FUNC_ALWAYS;
    }
    DXTypes::MapFailed("CompareOp", "D3D12_COMPARISON_FUNC");
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
    DXTypes::MapFailed("StencilOp", "D3D12_STENCIL_OP");
}

TextureFormat Unmap(const DXGI_FORMAT format)
{
    return DXTypes::Unmap(format);
}


} // /namespace D3D12Types

} // /namespace LLGL



// ================================================================================
