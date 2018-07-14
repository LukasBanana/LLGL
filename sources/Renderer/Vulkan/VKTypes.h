/*
 * VKTypes.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_VK_TYPES_H
#define LLGL_VK_TYPES_H


#include <vulkan/vulkan.h>
#include <LLGL/ShaderFlags.h>
#include <LLGL/PipelineLayoutFlags.h>
#include <LLGL/ResourceHeapFlags.h>
#include <LLGL/GraphicsPipelineFlags.h>
#include <LLGL/Format.h>
#include <LLGL/SamplerFlags.h>
#include <LLGL/QueryFlags.h>


namespace LLGL
{

namespace VKTypes
{


/* ----- Map functions ----- */

[[noreturn]]
void MapFailed(const std::string& typeName, const std::string& vknTypeName);

VkShaderStageFlagBits   Map( const ShaderType           shaderType        );
VkFormat                Map( const Format               format            );
VkImageViewType         Map( const TextureType          textureType       );
VkPrimitiveTopology     Map( const PrimitiveTopology    primitiveTopology );
VkPolygonMode           Map( const PolygonMode          polygonMode       );
VkCullModeFlags         Map( const CullMode             cullMode          );
VkCompareOp             Map( const CompareOp            compareOp         );
VkStencilOp             Map( const StencilOp            stencilOp         );
VkLogicOp               Map( const LogicOp              logicOp           );
VkBlendFactor           Map( const BlendOp              blendOp           );
VkBlendOp               Map( const BlendArithmetic      blendArithmetic   );
VkSamplerAddressMode    Map( const SamplerAddressMode   addressMode       );
VkDescriptorType        Map( const ResourceType         resourceViewType  );
VkQueryType             Map( const QueryType            queryType         );

Format                  Unmap( const VkFormat format );


/* ----- Convert functions ----- */

void Convert( VkViewport& dst, const Viewport& src );
void Convert( VkRect2D&   dst, const Scissor&  src );
void Convert( VkRect2D&   dst, const Viewport& src );


} // /namespace VKTypes

} // /namespace LLGL


#endif



// ================================================================================
