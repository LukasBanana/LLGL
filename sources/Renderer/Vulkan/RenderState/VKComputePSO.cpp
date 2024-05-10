/*
 * VKComputePSO.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "VKComputePSO.h"
#include "VKPipelineCache.h"
#include "../Shader/VKShader.h"
#include "../VKTypes.h"
#include "../VKCore.h"
#include "../../CheckedCast.h"
#include "../../PipelineStateUtils.h"
#include "../../../Core/StringUtils.h"
#include <LLGL/PipelineStateFlags.h>
#include <cstddef>


namespace LLGL
{


VKComputePSO::VKComputePSO(
    VkDevice                            device,
    const ComputePipelineDescriptor&    desc,
    PipelineCache*                      pipelineCache)
:
    VKPipelineState { device, VK_PIPELINE_BIND_POINT_COMPUTE, GetShadersAsArray(desc), desc.pipelineLayout }
{
    /* Create Vulkan compute pipeline object */
    if (VKPipelineCache* pipelineCacheVK = (pipelineCache != nullptr ? LLGL_CAST(VKPipelineCache*, pipelineCache) : nullptr))
        CreateVkPipeline(device, desc, pipelineCacheVK->GetNative());
    else
        CreateVkPipeline(device, desc);
}


/*
 * ======= Private: =======
 */

bool VKComputePSO::CreateVkPipeline(
    VkDevice                            device,
    const ComputePipelineDescriptor&    desc,
    VkPipelineCache                     pipelineCache)
{
    /* Get compute shader */
    VKShader* computeShaderVK = LLGL_CAST(VKShader*, desc.computeShader);
    if (computeShaderVK == nullptr)
    {
        GetMutableReport().Errorf("cannot create Vulkan compute pipeline without compute shader\n");
        return false;
    }

    const Report* computeShaderReport = computeShaderVK->GetReport();
    if (computeShaderReport != nullptr && computeShaderReport->HasErrors())
    {
        GetMutableReport().Errorf("Failed to load compute shader into Vulkan compute pipeline state [%s]\n", GetOptionalDebugName(desc.debugName));
        return false;
    }

    /* Get shader stages */
    VkPipelineShaderStageCreateInfo shaderStageCreateInfo;
    GetShaderCreateInfoAndOptionalPermutation(*computeShaderVK, shaderStageCreateInfo);

    /* Create graphics pipeline state object */
    VkComputePipelineCreateInfo createInfo;
    {
        createInfo.sType                = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        createInfo.pNext                = nullptr;
        createInfo.flags                = 0;
        createInfo.stage                = shaderStageCreateInfo;
        createInfo.layout               = GetVkPipelineLayout();
        createInfo.basePipelineHandle   = VK_NULL_HANDLE;
        createInfo.basePipelineIndex    = 0;
    }
    VkResult result = vkCreateComputePipelines(device, pipelineCache, 1, &createInfo, nullptr, ReleaseAndGetAddressOfVkPipeline());
    VKThrowIfFailed(result, "failed to create Vulkan compute pipeline");

    return true;
}


} // /namespace LLGL



// ================================================================================
