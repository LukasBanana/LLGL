/*
 * DXTypes.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
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
static void MapFailed(const std::string& typeName, const std::string& dxTypeName)
{
    throw std::invalid_argument("failed to map '" + typeName + "' to '" + dxTypeName + "' parameter");
}

[[noreturn]]
static void UnmapFailed(const std::string& typeName, const std::string& dxTypeName)
{
    throw std::invalid_argument("failed to unmap '" + typeName + "' from '" + dxTypeName + "' parameter");
}

DXGI_FORMAT Map(const VertexAttribute& attrib)
{
    switch (attrib.dataType)
    {
        case DataType::Float:
            switch (attrib.components)
            {
                case 1: return DXGI_FORMAT_R32_FLOAT;
                case 2: return DXGI_FORMAT_R32G32_FLOAT;
                case 3: return DXGI_FORMAT_R32G32B32_FLOAT;
                case 4: return DXGI_FORMAT_R32G32B32A32_FLOAT;
            }
            break;
        
        case DataType::Byte:
            if (attrib.conversion)
            {
                switch (attrib.components)
                {
                    case 1: return DXGI_FORMAT_R8_SNORM;
                    case 2: return DXGI_FORMAT_R8G8_SNORM;
                    case 4: return DXGI_FORMAT_R8G8B8A8_SNORM;
                }
            }
            else
            {
                switch (attrib.components)
                {
                    case 1: return DXGI_FORMAT_R8_SINT;
                    case 2: return DXGI_FORMAT_R8G8_SINT;
                    case 4: return DXGI_FORMAT_R8G8B8A8_SINT;
                }
            }
            break;
        
        case DataType::UByte:
            if (attrib.conversion)
            {
                switch (attrib.components)
                {
                    case 1: return DXGI_FORMAT_R8_UNORM;
                    case 2: return DXGI_FORMAT_R8G8_UNORM;
                    case 4: return DXGI_FORMAT_R8G8B8A8_UNORM;
                }
            }
            else
            {
                switch (attrib.components)
                {
                    case 1: return DXGI_FORMAT_R8_UINT;
                    case 2: return DXGI_FORMAT_R8G8_UINT;
                    case 4: return DXGI_FORMAT_R8G8B8A8_UINT;
                }
            }
            break;
        
        case DataType::Short:
            if (attrib.conversion)
            {
                switch (attrib.components)
                {
                    case 1: return DXGI_FORMAT_R16_SNORM;
                    case 2: return DXGI_FORMAT_R16G16_SNORM;
                    case 4: return DXGI_FORMAT_R16G16B16A16_SNORM;
                }
            }
            else
            {
                switch (attrib.components)
                {
                    case 1: return DXGI_FORMAT_R16_SINT;
                    case 2: return DXGI_FORMAT_R16G16_SINT;
                    case 4: return DXGI_FORMAT_R16G16B16A16_SINT;
                }
            }
            break;
        
        case DataType::UShort:
            if (attrib.conversion)
            {
                switch (attrib.components)
                {
                    case 1: return DXGI_FORMAT_R16_UNORM;
                    case 2: return DXGI_FORMAT_R16G16_UNORM;
                    case 4: return DXGI_FORMAT_R16G16B16A16_UNORM;
                }
            }
            else
            {
                switch (attrib.components)
                {
                    case 1: return DXGI_FORMAT_R16_UINT;
                    case 2: return DXGI_FORMAT_R16G16_UINT;
                    case 4: return DXGI_FORMAT_R16G16B16A16_UINT;
                }
            }
            break;
        
        case DataType::Int:
            switch (attrib.components)
            {
                case 1: return DXGI_FORMAT_R32_SINT;
                case 2: return DXGI_FORMAT_R32G32_SINT;
                case 3: return DXGI_FORMAT_R32G32B32_SINT;
                case 4: return DXGI_FORMAT_R32G32B32A32_SINT;
            }
            break;

        case DataType::UInt:
            switch (attrib.components)
            {
                case 1: return DXGI_FORMAT_R32_UINT;
                case 2: return DXGI_FORMAT_R32G32_UINT;
                case 3: return DXGI_FORMAT_R32G32B32_UINT;
                case 4: return DXGI_FORMAT_R32G32B32A32_UINT;
            }
            break;
    }
    MapFailed("VertexAttribute", "DXGI_FORMAT");
}

D3D12_FILL_MODE Map(const PolygonMode polygonMode)
{
    switch (polygonMode)
    {
        case PolygonMode::Fill:         return D3D12_FILL_MODE_SOLID;
        case PolygonMode::Wireframe:    return D3D12_FILL_MODE_WIREFRAME;
        case PolygonMode::Points:       throw std::invalid_argument("'PolygonMode::Points' can not be mapped to 'D3D12_FILL_MODE'");
    }
    MapFailed("PolygonMode", "D3D12_FILL_MODE");
}

D3D12_CULL_MODE Map(const CullMode cullMode)
{
    switch (cullMode)
    {
        case CullMode::Disabled:    return D3D12_CULL_MODE_NONE;
        case CullMode::Front:       return D3D12_CULL_MODE_FRONT;
        case CullMode::Back:        return D3D12_CULL_MODE_BACK;
    }
    MapFailed("CullMode", "D3D12_CULL_MODE");
}

// see https://msdn.microsoft.com/en-us/library/windows/desktop/dn770338(v=vs.85).aspx
D3D12_BLEND Map(const BlendOp blendOp)
{
    switch (blendOp)
    {
        case BlendOp::Zero:         return D3D12_BLEND_ZERO;
        case BlendOp::One:          return D3D12_BLEND_ONE;
        case BlendOp::SrcColor:     return D3D12_BLEND_SRC_COLOR;
        case BlendOp::InvSrcColor:  return D3D12_BLEND_INV_SRC_COLOR;
        case BlendOp::SrcAlpha:     return D3D12_BLEND_SRC_ALPHA;
        case BlendOp::InvSrcAlpha:  return D3D12_BLEND_INV_SRC_ALPHA;
        case BlendOp::DestColor:    return D3D12_BLEND_DEST_COLOR;
        case BlendOp::InvDestColor: return D3D12_BLEND_INV_DEST_COLOR;
        case BlendOp::DestAlpha:    return D3D12_BLEND_DEST_ALPHA;
        case BlendOp::InvDestAlpha: return D3D12_BLEND_INV_DEST_ALPHA;
                                    /*return D3D12_BLEND_SRC_ALPHA_SAT;
                                    return D3D12_BLEND_BLEND_FACTOR;
                                    return D3D12_BLEND_INV_BLEND_FACTOR;
                                    return D3D12_BLEND_SRC1_COLOR;
                                    return D3D12_BLEND_INV_SRC1_COLOR;
                                    return D3D12_BLEND_SRC1_ALPHA;
                                    return D3D12_BLEND_INV_SRC1_ALPHA;*/
    }
    MapFailed("BlendOp", "D3D12_BLEND");
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
    MapFailed("BlendArithmetic", "D3D12_BLEND_OP");
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
    MapFailed("CompareOp", "D3D12_COMPARISON_FUNC");
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
    MapFailed("StencilOp", "D3D12_STENCIL_OP");
}


} // /namespace DXTypes

} // /namespace LLGL



// ================================================================================
