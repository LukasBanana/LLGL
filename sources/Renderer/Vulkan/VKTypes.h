/*
 * VKTypes.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_VK_TYPES_H
#define LLGL_VK_TYPES_H


#include <vulkan/vulkan.h>
#include <LLGL/ShaderFlags.h>
#include <LLGL/PipelineLayoutFlags.h>
#include <LLGL/ResourceHeapFlags.h>
#include <LLGL/PipelineStateFlags.h>
#include <LLGL/RenderPassFlags.h>
#include <LLGL/Format.h>
#include <LLGL/SamplerFlags.h>
#include <LLGL/QueryHeapFlags.h>


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
VkQueryType             Map( const QueryType            queryType         );
VkAttachmentLoadOp      Map( const AttachmentLoadOp     loadOp            );
VkAttachmentStoreOp     Map( const AttachmentStoreOp    storeOp           );
VkStencilFaceFlags      Map( const StencilFace          stencilFace       );
VkPipelineBindPoint     Map( const PipelineBindPoint    pipelineBindPoint );

VkIndexType             ToVkIndexType(const Format format);
VkSampleCountFlagBits   ToVkSampleCountBits(std::uint32_t samples);
VkOffset3D              ToVkOffset(const Offset3D& offset);
VkExtent3D              ToVkExtent(const Extent3D& extent);

Format                  Unmap( const VkFormat format );


/* ----- Convert functions ----- */

void Convert( VkViewport& dst, const Viewport& src );
void Convert( VkRect2D&   dst, const Scissor&  src );
void Convert( VkRect2D&   dst, const Viewport& src );


} // /namespace VKTypes

} // /namespace LLGL


#endif



// ================================================================================
