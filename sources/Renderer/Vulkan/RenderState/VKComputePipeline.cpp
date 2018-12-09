/*
 * VKComputePipeline.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKComputePipeline.h"
#include "../Shader/VKShaderProgram.h"
#include "VKPipelineLayout.h"
#include "../VKTypes.h"
#include "../VKCore.h"
#include "../../CheckedCast.h"
#include <cstddef>


namespace LLGL
{


VKComputePipeline::VKComputePipeline(
    const VKPtr<VkDevice>& device, const ComputePipelineDescriptor& desc, VkPipelineLayout defaultPipelineLayout) :
        device_         { device                    },
        pipelineLayout_ { defaultPipelineLayout     },
        pipeline_       { device, vkDestroyPipeline }
{
    /* Get pipeline layout object */
    if (desc.pipelineLayout)
    {
        auto pipelineLayoutVK = LLGL_CAST(const VKPipelineLayout*, desc.pipelineLayout);
        pipelineLayout_ = pipelineLayoutVK->GetVkPipelineLayout();
    }

    /* Create Vulkan compute pipeline object */
    CreateComputePipeline(desc);
}


/*
 * ======= Private: =======
 */

void VKComputePipeline::CreateComputePipeline(const ComputePipelineDescriptor& desc)
{
    /* Get shader program object */
    auto shaderProgramVK = LLGL_CAST(const VKShaderProgram*, desc.shaderProgram);
    if (!shaderProgramVK)
        throw std::invalid_argument("failed to create compute pipeline due to missing shader program");

    /* Get shader stages */
    auto shaderStageCreateInfos = shaderProgramVK->GetShaderStageCreateInfos();
    if (shaderStageCreateInfos.size() != 1)
        throw std::invalid_argument("invalid number of shader stages for Vulkan compute pipeline");

    /* Create graphics pipeline state object */
    VkComputePipelineCreateInfo createInfo;
    {
        createInfo.sType                = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        createInfo.pNext                = nullptr;
        createInfo.flags                = 0;
        createInfo.stage                = shaderStageCreateInfos.front();
        createInfo.layout               = pipelineLayout_;
        createInfo.basePipelineHandle   = VK_NULL_HANDLE;
        createInfo.basePipelineIndex    = 0;
    }
    auto result = vkCreateComputePipelines(device_, VK_NULL_HANDLE, 1, &createInfo, nullptr, pipeline_.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Vulkan compute pipeline");
}


} // /namespace LLGL



// ================================================================================
