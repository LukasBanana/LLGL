/*
 * VKTypes.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKTypes.h"
#include <stdexcept>
#include <string>


namespace LLGL
{

namespace VKTypes
{


/* ----- Internal functions ----- */

[[noreturn]]
void MapFailed(const std::string& typeName, const std::string& vknTypeName)
{
    throw std::invalid_argument("failed to map '" + typeName + "' to '" + vknTypeName + "' Vulkan parameter");
}


/* ----- Map functions ----- */

VkShaderStageFlagBits Map(const ShaderType shaderType)
{
    switch (shaderType)
    {
        case ShaderType::Vertex:            return VK_SHADER_STAGE_VERTEX_BIT;
        case ShaderType::TessControl:       return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        case ShaderType::TessEvaluation:    return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
        case ShaderType::Geometry:          return VK_SHADER_STAGE_GEOMETRY_BIT;
        case ShaderType::Fragment:          return VK_SHADER_STAGE_FRAGMENT_BIT;
        case ShaderType::Compute:           return VK_SHADER_STAGE_COMPUTE_BIT;
    }
    MapFailed("ShaderType", "VkShaderStageFlagBits");
}

VkFormat Map(const VectorType vectorType)
{
    switch (vectorType)
    {
        case VectorType::Float:     return VK_FORMAT_R32_SFLOAT;
        case VectorType::Float2:    return VK_FORMAT_R32G32_SFLOAT;
        case VectorType::Float3:    return VK_FORMAT_R32G32B32_SFLOAT;
        case VectorType::Float4:    return VK_FORMAT_R32G32B32A32_SFLOAT;
        case VectorType::Double:    return VK_FORMAT_R64_SFLOAT;
        case VectorType::Double2:   return VK_FORMAT_R64G64_SFLOAT;
        case VectorType::Double3:   return VK_FORMAT_R64G64B64_SFLOAT;
        case VectorType::Double4:   return VK_FORMAT_R64G64B64A64_SFLOAT;
        case VectorType::Int:       return VK_FORMAT_R32_SINT;
        case VectorType::Int2:      return VK_FORMAT_R32G32_SINT;
        case VectorType::Int3:      return VK_FORMAT_R32G32B32_SINT;
        case VectorType::Int4:      return VK_FORMAT_R32G32B32A32_SINT;
        case VectorType::UInt:      return VK_FORMAT_R32_UINT;
        case VectorType::UInt2:     return VK_FORMAT_R32G32_UINT;
        case VectorType::UInt3:     return VK_FORMAT_R32G32B32_UINT;
        case VectorType::UInt4:     return VK_FORMAT_R32G32B32A32_UINT;
    }
    MapFailed("VectorType", "VkFormat");
}

VkPrimitiveTopology Map(const PrimitiveTopology primitiveTopology)
{
    switch (primitiveTopology)
    {
        case PrimitiveTopology::PointList:              return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        case PrimitiveTopology::LineList:               return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        case PrimitiveTopology::LineStrip:              return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
        case PrimitiveTopology::LineLoop:               break;
        case PrimitiveTopology::LineListAdjacency:      return VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY;
        case PrimitiveTopology::LineStripAdjacency:     return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY;
        case PrimitiveTopology::TriangleList:           return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        case PrimitiveTopology::TriangleStrip:          return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
        case PrimitiveTopology::TriangleFan:            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
        case PrimitiveTopology::TriangleListAdjacency:  return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY;
        case PrimitiveTopology::TriangleStripAdjacency: return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY;
        default:
            if (primitiveTopology >= PrimitiveTopology::Patches1 && primitiveTopology <= PrimitiveTopology::Patches32)
                return VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
            break;
    }
    MapFailed("PrimitiveTopology", "VkPrimitiveTopology");
}

VkPolygonMode Map(const PolygonMode polygonMode)
{
    switch (polygonMode)
    {
        case PolygonMode::Fill:         return VK_POLYGON_MODE_FILL;
        case PolygonMode::Wireframe:    return VK_POLYGON_MODE_LINE;
        case PolygonMode::Points:       return VK_POLYGON_MODE_POINT;
    }
    MapFailed("PolygonMode", "VkPolygonMode");
}

VkCullModeFlags Map(const CullMode cullMode)
{
    switch (cullMode)
    {
        case CullMode::Disabled:    return VK_CULL_MODE_NONE;
        case CullMode::Front:       return VK_CULL_MODE_FRONT_BIT;
        case CullMode::Back:        return VK_CULL_MODE_BACK_BIT;
    }
    MapFailed("CullMode", "VkCullModeFlags");
}

VkCompareOp Map(const CompareOp compareOp)
{
    switch (compareOp)
    {
        case CompareOp::Never:          return VK_COMPARE_OP_NEVER;
        case CompareOp::Less:           return VK_COMPARE_OP_LESS;
        case CompareOp::Equal:          return VK_COMPARE_OP_EQUAL;
        case CompareOp::LessEqual:      return VK_COMPARE_OP_LESS_OR_EQUAL;
        case CompareOp::Greater:        return VK_COMPARE_OP_GREATER;
        case CompareOp::NotEqual:       return VK_COMPARE_OP_NOT_EQUAL;
        case CompareOp::GreaterEqual:   return VK_COMPARE_OP_GREATER_OR_EQUAL;
        case CompareOp::Ever:           return VK_COMPARE_OP_ALWAYS;
    }
    MapFailed("CompareOp", "VkCompareOp");
}

VkStencilOp Map(const StencilOp stencilOp)
{
    switch (stencilOp)
    {
        case StencilOp::Keep:       return VK_STENCIL_OP_KEEP;
        case StencilOp::Zero:       return VK_STENCIL_OP_ZERO;
        case StencilOp::Replace:    return VK_STENCIL_OP_REPLACE;
        case StencilOp::IncClamp:   return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
        case StencilOp::DecClamp:   return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
        case StencilOp::Invert:     return VK_STENCIL_OP_INVERT;
        case StencilOp::IncWrap:    return VK_STENCIL_OP_INCREMENT_AND_WRAP;
        case StencilOp::DecWrap:    return VK_STENCIL_OP_DECREMENT_AND_WRAP;
    }
    MapFailed("StencilOp", "VkStencilOp");
}

VkLogicOp Map(const LogicOp logicOp)
{
    switch (logicOp)
    {
        case LogicOp::Disabled:     break;
        case LogicOp::Clear:        return VK_LOGIC_OP_CLEAR;
        case LogicOp::Set:          return VK_LOGIC_OP_SET;
        case LogicOp::Copy:         return VK_LOGIC_OP_COPY;
        case LogicOp::CopyInverted: return VK_LOGIC_OP_COPY_INVERTED;
        case LogicOp::NoOp:         return VK_LOGIC_OP_NO_OP;
        case LogicOp::Invert:       return VK_LOGIC_OP_INVERT;
        case LogicOp::AND:          return VK_LOGIC_OP_AND;
        case LogicOp::ANDReverse:   return VK_LOGIC_OP_AND_REVERSE;
        case LogicOp::ANDInverted:  return VK_LOGIC_OP_AND_INVERTED;
        case LogicOp::NAND:         return VK_LOGIC_OP_NAND;
        case LogicOp::OR:           return VK_LOGIC_OP_OR;
        case LogicOp::ORReverse:    return VK_LOGIC_OP_OR_REVERSE;
        case LogicOp::ORInverted:   return VK_LOGIC_OP_OR_INVERTED;
        case LogicOp::NOR:          return VK_LOGIC_OP_NOR;
        case LogicOp::XOR:          return VK_LOGIC_OP_XOR;
        case LogicOp::Equiv:        return VK_LOGIC_OP_EQUIVALENT;
    }
    MapFailed("LogicOp", "VkLogicOp");
}

VkBlendFactor Map(const BlendOp blendOp)
{
    switch (blendOp)
    {
        case BlendOp::Zero:             return VK_BLEND_FACTOR_ZERO;
        case BlendOp::One:              return VK_BLEND_FACTOR_ONE;
        case BlendOp::SrcColor:         return VK_BLEND_FACTOR_SRC_COLOR;
        case BlendOp::InvSrcColor:      return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
        case BlendOp::SrcAlpha:         return VK_BLEND_FACTOR_SRC_ALPHA;
        case BlendOp::InvSrcAlpha:      return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        case BlendOp::DestColor:        return VK_BLEND_FACTOR_DST_COLOR;
        case BlendOp::InvDestColor:     return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
        case BlendOp::DestAlpha:        return VK_BLEND_FACTOR_DST_ALPHA;
        case BlendOp::InvDestAlpha:     return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
        case BlendOp::SrcAlphaSaturate: return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
        case BlendOp::BlendFactor:      return VK_BLEND_FACTOR_CONSTANT_COLOR;              // VK_BLEND_FACTOR_CONSTANT_ALPHA ???
        case BlendOp::InvBlendFactor:   return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;    // VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA ???
        case BlendOp::Src1Color:        return VK_BLEND_FACTOR_SRC1_COLOR;
        case BlendOp::InvSrc1Color:     return VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
        case BlendOp::Src1Alpha:        return VK_BLEND_FACTOR_SRC1_ALPHA;
        case BlendOp::InvSrc1Alpha:     return VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
    }
    MapFailed("BlendOp", "VkBlendFactor");
}

VkBlendOp Map(const BlendArithmetic blendArithmetic)
{
    switch (blendArithmetic)
    {
        case BlendArithmetic::Add:          return VK_BLEND_OP_ADD;
        case BlendArithmetic::Subtract:     return VK_BLEND_OP_SUBTRACT;
        case BlendArithmetic::RevSubtract:  return VK_BLEND_OP_REVERSE_SUBTRACT;
        case BlendArithmetic::Min:          return VK_BLEND_OP_MIN;
        case BlendArithmetic::Max:          return VK_BLEND_OP_MAX;
    }
    MapFailed("BlendArithmetic", "VkBlendOp");
}

VkSamplerAddressMode Map(const TextureWrap textureWrap)
{
    switch (textureWrap)
    {
        case TextureWrap::Repeat:       return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        case TextureWrap::Mirror:       return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        case TextureWrap::Clamp:        return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        case TextureWrap::Border:       return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        case TextureWrap::MirrorOnce:   return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
    }
    MapFailed("TextureWrap", "VkSamplerAddressMode");
}

VkDescriptorType Map(const LayoutBindingType layoutingBindingType)
{
    switch (layoutingBindingType)
    {
        case LayoutBindingType::Sampler:        return VK_DESCRIPTOR_TYPE_SAMPLER;
        case LayoutBindingType::Texture:        return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        case LayoutBindingType::ConstantBuffer: return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        case LayoutBindingType::StorageBuffer:  return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    }
    MapFailed("LayoutBindingType", "VkDescriptorType");
}


} // /namespace VKTypes

} // /namespace LLGL



// ================================================================================
