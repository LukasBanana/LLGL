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


} // /namespace VKTypes

} // /namespace LLGL



// ================================================================================
