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
#include <LLGL/GraphicsPipelineFlags.h>
#include <LLGL/Format.h>


namespace LLGL
{

namespace VKTypes
{


VkShaderStageFlagBits Map( const ShaderType        shaderType        );
VkFormat              Map( const VectorType        vectorType        );
VkPrimitiveTopology   Map( const PrimitiveTopology primitiveTopology );


} // /namespace VKTypes

} // /namespace LLGL


#endif



// ================================================================================
