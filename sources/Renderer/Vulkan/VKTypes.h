/*
 * VKTypes.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_VK_TYPES_H
#define LLGL_VK_TYPES_H


#include "Vulkan.h"
#include <LLGL/ShaderFlags.h>
#include <LLGL/PipelineLayoutFlags.h>
#include <LLGL/GraphicsPipelineFlags.h>
#include <LLGL/Format.h>
#include <LLGL/SamplerFlags.h>
#include <LLGL/QueryFlags.h>


namespace LLGL
{

namespace VKTypes
{


[[noreturn]]
void MapFailed(const std::string& typeName, const std::string& vknTypeName);

VkShaderStageFlagBits   Map( const ShaderType        shaderType           );
VkFormat                Map( const VectorType        vectorType           );
VkPrimitiveTopology     Map( const PrimitiveTopology primitiveTopology    );
VkPolygonMode           Map( const PolygonMode       polygonMode          );
VkCullModeFlags         Map( const CullMode          cullMode             );
VkCompareOp             Map( const CompareOp         compareOp            );
VkStencilOp             Map( const StencilOp         stencilOp            );
VkLogicOp               Map( const LogicOp           logicOp              );
VkBlendFactor           Map( const BlendOp           blendOp              );
VkBlendOp               Map( const BlendArithmetic   blendArithmetic      );
VkSamplerAddressMode    Map( const TextureWrap       textureWrap          );
VkDescriptorType        Map( const LayoutBindingType layoutingBindingType );
VkQueryType             Map( const QueryType         queryType            );


} // /namespace VKTypes

} // /namespace LLGL


#endif



// ================================================================================
