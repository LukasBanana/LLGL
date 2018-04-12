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

VkFormat Map(const TextureFormat textureFormat)
{
    switch (textureFormat)
    {
        case TextureFormat::Unknown:        break;

        /* --- Base internal formats --- */
        case TextureFormat::DepthComponent: return VK_FORMAT_D32_SFLOAT;
        case TextureFormat::DepthStencil:   return VK_FORMAT_D24_UNORM_S8_UINT;
        case TextureFormat::R:              return VK_FORMAT_R8_UNORM;
        case TextureFormat::RG:             return VK_FORMAT_R8G8_UNORM;
        case TextureFormat::RGB:            return VK_FORMAT_R8G8B8_UNORM;
        case TextureFormat::RGBA:           return VK_FORMAT_R8G8B8A8_UNORM;

        /* --- Sized internal formats --- */
        case TextureFormat::R8:             return VK_FORMAT_R8_UNORM;
        case TextureFormat::R8Sgn:          return VK_FORMAT_R8_SNORM;

        case TextureFormat::R16:            return VK_FORMAT_R16_UNORM;
        case TextureFormat::R16Sgn:         return VK_FORMAT_R16_SNORM;
        case TextureFormat::R16Float:       return VK_FORMAT_R16_SFLOAT;

        case TextureFormat::R32UInt:        return VK_FORMAT_R32_UINT;
        case TextureFormat::R32SInt:        return VK_FORMAT_R32_SINT;
        case TextureFormat::R32Float:       return VK_FORMAT_R32_SFLOAT;

        case TextureFormat::RG8:            return VK_FORMAT_R8G8_UNORM;
        case TextureFormat::RG8Sgn:         return VK_FORMAT_R8G8_SNORM;

        case TextureFormat::RG16:           return VK_FORMAT_R16G16_UNORM;
        case TextureFormat::RG16Sgn:        return VK_FORMAT_R16G16_SNORM;
        case TextureFormat::RG16Float:      return VK_FORMAT_R16G16_SFLOAT;

        case TextureFormat::RG32UInt:       return VK_FORMAT_R32G32_UINT;
        case TextureFormat::RG32SInt:       return VK_FORMAT_R32G32_SINT;
        case TextureFormat::RG32Float:      return VK_FORMAT_R32G32_SFLOAT;

        case TextureFormat::RGB8:           return VK_FORMAT_R8G8B8_UNORM;
        case TextureFormat::RGB8Sgn:        return VK_FORMAT_R8G8B8_SNORM;

        case TextureFormat::RGB16:          return VK_FORMAT_R16G16B16_UNORM;
        case TextureFormat::RGB16Sgn:       return VK_FORMAT_R16G16B16_SNORM;
        case TextureFormat::RGB16Float:     return VK_FORMAT_R16G16B16_SFLOAT;

        case TextureFormat::RGB32UInt:      return VK_FORMAT_R32G32B32_UINT;
        case TextureFormat::RGB32SInt:      return VK_FORMAT_R32G32B32_SINT;
        case TextureFormat::RGB32Float:     return VK_FORMAT_R32G32B32_SFLOAT;

        case TextureFormat::RGBA8:          return VK_FORMAT_R8G8B8A8_UNORM;
        case TextureFormat::RGBA8Sgn:       return VK_FORMAT_R8G8B8A8_SNORM;

        case TextureFormat::RGBA16:         return VK_FORMAT_R16G16B16A16_UNORM;
        case TextureFormat::RGBA16Sgn:      return VK_FORMAT_R16G16B16A16_SNORM;
        case TextureFormat::RGBA16Float:    return VK_FORMAT_R16G16B16A16_SFLOAT;

        case TextureFormat::RGBA32UInt:     return VK_FORMAT_R32G32B32A32_UINT;
        case TextureFormat::RGBA32SInt:     return VK_FORMAT_R32G32B32A32_SINT;
        case TextureFormat::RGBA32Float:    return VK_FORMAT_R32G32B32A32_SFLOAT;

        /* --- Compressed formats --- */
        case TextureFormat::RGB_DXT1:       return VK_FORMAT_BC1_RGB_UNORM_BLOCK;
        case TextureFormat::RGBA_DXT1:      return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
        case TextureFormat::RGBA_DXT3:      return VK_FORMAT_BC2_UNORM_BLOCK;
        case TextureFormat::RGBA_DXT5:      return VK_FORMAT_BC3_UNORM_BLOCK;
    }
    MapFailed("TextureFormat", "VkFormat");
}

VkImageType Map(const TextureType textureType)
{
    switch (textureType)
    {
        case TextureType::Texture1D:        return VK_IMAGE_TYPE_1D;
        case TextureType::Texture2D:        return VK_IMAGE_TYPE_2D;
        case TextureType::Texture3D:        return VK_IMAGE_TYPE_3D;
        case TextureType::TextureCube:      return VK_IMAGE_TYPE_3D;
        case TextureType::Texture1DArray:   return VK_IMAGE_TYPE_2D;
        case TextureType::Texture2DArray:   return VK_IMAGE_TYPE_3D;
        case TextureType::TextureCubeArray: return VK_IMAGE_TYPE_3D;
        case TextureType::Texture2DMS:      return VK_IMAGE_TYPE_2D;
        case TextureType::Texture2DMSArray: return VK_IMAGE_TYPE_3D;
    }
    MapFailed("TextureType", "VkImageType");
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

VkDescriptorType Map(ResourceViewType resourceViewType)
{
    switch (resourceViewType)
    {
        case ResourceViewType::Sampler:         return VK_DESCRIPTOR_TYPE_SAMPLER;
        case ResourceViewType::Texture:         return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        case ResourceViewType::ConstantBuffer:  return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        case ResourceViewType::StorageBuffer:   return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    }
    MapFailed("LayoutBindingType", "VkDescriptorType");
}

VkQueryType Map(const QueryType queryType)
{
    switch (queryType)
    {
        case QueryType::SamplesPassed:                  /* pass */
        case QueryType::AnySamplesPassed:               /* pass */
        case QueryType::AnySamplesPassedConservative:   return VK_QUERY_TYPE_OCCLUSION;
        case QueryType::TimeElapsed:                    return VK_QUERY_TYPE_TIMESTAMP;
        case QueryType::StreamOutPrimitivesWritten:     break; // ???
        case QueryType::StreamOutOverflow:              break; // ???
        case QueryType::PipelineStatistics:             return VK_QUERY_TYPE_PIPELINE_STATISTICS;
    }
    MapFailed("QueryType", "VkQueryType");
}


} // /namespace VKTypes

} // /namespace LLGL



// ================================================================================
