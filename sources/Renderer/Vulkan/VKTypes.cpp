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
void MapFailed(const std::string& typeName, const std::string& dxTypeName)
{
    throw std::invalid_argument("failed to map '" + typeName + "' to '" + dxTypeName + "' Vulkan parameter");
}


/* ----- Map functions ----- */

VkShaderStageFlagBits Map( const ShaderType shaderType )
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


} // /namespace VKTypes

} // /namespace LLGL



// ================================================================================
