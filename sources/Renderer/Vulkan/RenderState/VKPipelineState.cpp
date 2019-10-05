/*
 * VKPipelineState.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKPipelineState.h"
#include "VKPipelineLayout.h"
#include "../../CheckedCast.h"


namespace LLGL
{


VKPipelineState::VKPipelineState(const VKPtr<VkDevice>& device, VkPipelineBindPoint bindPoint) :
    pipeline_  { device, vkDestroyPipeline },
    bindPoint_ { bindPoint                 }
{
}


/*
 * ======= Protected: =======
 */

VkPipelineLayout VKPipelineState::GetVkPipelineLayoutOrDefault(
    const PipelineLayout*   pipelineLayout,
    VkPipelineLayout        defaultPipelineLayout)
{
    if (pipelineLayout)
    {
        auto pipelineLayoutVK = LLGL_CAST(const VKPipelineLayout*, pipelineLayout);
        return pipelineLayoutVK->GetVkPipelineLayout();
    }
    return defaultPipelineLayout;
}

VkPipeline* VKPipelineState::GetVkPipelineAddress()
{
    return pipeline_.ReleaseAndGetAddressOf();
}


} // /namespace LLGL



// ================================================================================
