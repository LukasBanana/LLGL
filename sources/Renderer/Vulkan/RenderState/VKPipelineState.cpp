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


VKPipelineState::VKPipelineState(
    const VKPtr<VkDevice>&  device,
    VkPipelineBindPoint     bindPoint,
    const PipelineLayout*   pipelineLayout)
:
    pipeline_  { device, vkDestroyPipeline },
    bindPoint_ { bindPoint                 }
{
    if (pipelineLayout != nullptr)
        pipelineLayout_ = LLGL_CAST(const VKPipelineLayout*, pipelineLayout);
}

const Report* VKPipelineState::GetReport() const
{
    return nullptr; //TODO
}

void VKPipelineState::BindPipeline(VkCommandBuffer commandBuffer)
{
    vkCmdBindPipeline(commandBuffer, GetBindPoint(), GetVkPipeline());
    if (pipelineLayout_ != nullptr)
        pipelineLayout_->BindStaticDescriptorSets(commandBuffer, GetBindPoint());
}


/*
 * ======= Protected: =======
 */

VkPipeline* VKPipelineState::ReleaseAndGetAddressOfVkPipeline()
{
    return pipeline_.ReleaseAndGetAddressOf();
}

VkPipelineLayout VKPipelineState::GetVkPipelineLayoutOrDefault(VkPipelineLayout defaultPipelineLayout) const
{
    return (pipelineLayout_ != nullptr ? pipelineLayout_->GetVkPipelineLayout() : defaultPipelineLayout);
}


} // /namespace LLGL



// ================================================================================
