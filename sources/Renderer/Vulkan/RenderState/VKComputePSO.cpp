/*
 * VKComputePSO.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKComputePSO.h"
#include "../Shader/VKShader.h"
#include "../VKTypes.h"
#include "../VKCore.h"
#include "../../CheckedCast.h"
#include <LLGL/PipelineStateFlags.h>
#include <cstddef>


namespace LLGL
{


VKComputePSO::VKComputePSO(
    VkDevice                            device,
    const ComputePipelineDescriptor&    desc,
    VkPipelineLayout                    defaultPipelineLayout)
:
    VKPipelineState { device, VK_PIPELINE_BIND_POINT_COMPUTE, desc.pipelineLayout }
{
    /* Create Vulkan compute pipeline object */
    CreateVkPipeline(device, GetVkPipelineLayoutOrDefault(defaultPipelineLayout), desc);
}


/*
 * ======= Private: =======
 */

void VKComputePSO::CreateVkPipeline(
    VkDevice                            device,
    VkPipelineLayout                    pipelineLayout,
    const ComputePipelineDescriptor&    desc)
{
    /* Get compute shader */
    auto computeShaderVK = LLGL_CAST(const VKShader*, desc.computeShader);
    if (!computeShaderVK)
        throw std::invalid_argument("cannot create Vulkan compute pipeline without compute shader");

    /* Get shader stages */
    VkPipelineShaderStageCreateInfo shaderStageCreateInfo;
    computeShaderVK->FillShaderStageCreateInfo(shaderStageCreateInfo);

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
    auto result = vkCreateComputePipelines(device, VK_NULL_HANDLE, 1, &createInfo, nullptr, ReleaseAndGetAddressOfVkPipeline());
    VKThrowIfFailed(result, "failed to create Vulkan compute pipeline");
}


} // /namespace LLGL



// ================================================================================
