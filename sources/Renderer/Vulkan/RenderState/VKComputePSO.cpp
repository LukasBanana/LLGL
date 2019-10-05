/*
 * VKComputePSO.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKComputePSO.h"
#include "../Shader/VKShaderProgram.h"
#include "../VKTypes.h"
#include "../VKCore.h"
#include "../../CheckedCast.h"
#include <LLGL/PipelineStateFlags.h>
#include <cstddef>


namespace LLGL
{


VKComputePSO::VKComputePSO(
    const VKPtr<VkDevice>&              device,
    const ComputePipelineDescriptor&    desc,
    VkPipelineLayout                    defaultPipelineLayout)
:
    VKPipelineState { device, VK_PIPELINE_BIND_POINT_COMPUTE }
{
    /* Create Vulkan compute pipeline object */
    CreateVkPipeline(
        device,
        GetVkPipelineLayoutOrDefault(desc.pipelineLayout, defaultPipelineLayout),
        desc
    );
}


/*
 * ======= Private: =======
 */

void VKComputePSO::CreateVkPipeline(
    VkDevice                            device,
    VkPipelineLayout                    pipelineLayout,
    const ComputePipelineDescriptor&    desc)
{
    /* Get shader program object */
    auto shaderProgramVK = LLGL_CAST(const VKShaderProgram*, desc.shaderProgram);
    if (!shaderProgramVK)
        throw std::invalid_argument("failed to create compute pipeline due to missing shader program");

    /* Get shader stages */
    std::uint32_t shaderStageCount = 1;
    VkPipelineShaderStageCreateInfo shaderStageCreateInfo;
    shaderProgramVK->FillShaderStageCreateInfos(&shaderStageCreateInfo, shaderStageCount);

    if (shaderStageCount != 1)
        throw std::invalid_argument("invalid number of shader stages for Vulkan compute pipeline");

    /* Create graphics pipeline state object */
    VkComputePipelineCreateInfo createInfo;
    {
        createInfo.sType                = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        createInfo.pNext                = nullptr;
        createInfo.flags                = 0;
        createInfo.stage                = shaderStageCreateInfo;
        createInfo.layout               = pipelineLayout;
        createInfo.basePipelineHandle   = VK_NULL_HANDLE;
        createInfo.basePipelineIndex    = 0;
    }
    auto result = vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &createInfo, nullptr, GetVkPipelineAddress());
    VKThrowIfFailed(result, "failed to create Vulkan compute pipeline");
}


} // /namespace LLGL



// ================================================================================
