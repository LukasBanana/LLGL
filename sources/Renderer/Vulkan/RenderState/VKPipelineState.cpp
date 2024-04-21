/*
 * VKPipelineState.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "VKPipelineState.h"
#include "VKPipelineLayout.h"
#include "../Shader/VKShader.h"
#include "../Shader/VKShaderModulePool.h"
#include "../../CheckedCast.h"


namespace LLGL
{


VKPipelineState::VKPipelineState(
    VkDevice                    device,
    VkPipelineBindPoint         bindPoint,
    const ArrayView<Shader*>&   shaders,
    const PipelineLayout*       pipelineLayout)
:
    pipeline_  { device, vkDestroyPipeline },
    bindPoint_ { bindPoint                 }
{
    if (pipelineLayout != nullptr)
    {
        pipelineLayout_ = LLGL_CAST(const VKPipelineLayout*, pipelineLayout);
        if (pipelineLayout_->GetNumUniforms() > 0)
            pipelineLayoutPerm_ = pipelineLayout_->CreateVkPipelineLayoutPermutation(device, shaders, uniformRanges_);
    }
}

const Report* VKPipelineState::GetReport() const
{
    return (*report_.GetText() != '\0' || report_.HasErrors() ? &report_ : nullptr);
}

void VKPipelineState::BindPipelineAndStaticDescriptorSet(VkCommandBuffer commandBuffer)
{
    vkCmdBindPipeline(commandBuffer, GetBindPoint(), GetVkPipeline());

    if (pipelineLayout_ != nullptr)
    {
        VkDescriptorSet staticDescriptorSet = pipelineLayout_->GetStaticDescriptorSet();
        if (staticDescriptorSet != VK_NULL_HANDLE)
        {
            vkCmdBindDescriptorSets(
                /*commandBuffer:*/      commandBuffer,
                /*pipelineBindPoint:*/  GetBindPoint(),
                /*layout:*/             GetVkPipelineLayout(),
                /*firstSet:*/           pipelineLayout_->GetBindPointForImmutableSamplers(),
                /*descriptorSetCount:*/ 1,
                /*pDescriptorSets:*/    &staticDescriptorSet,
                /*dynamicOffsetCount:*/ 0,
                /*pDynamicOffsets*/     nullptr
            );
        }
    }
}

//private
void VKPipelineState::BindDescriptorSets(
    VkCommandBuffer         commandBuffer,
    std::uint32_t           firstSet,
    std::uint32_t           descriptorSetCount,
    const VkDescriptorSet*  descriptorSets)
{
    vkCmdBindDescriptorSets(
        /*commandBuffer:*/      commandBuffer,
        /*pipelineBindPoint:*/  GetBindPoint(),
        /*layout:*/             GetVkPipelineLayout(),
        /*firstSet:*/           firstSet,
        /*descriptorSetCount:*/ descriptorSetCount,
        /*pDescriptorSets:*/    descriptorSets,
        /*dynamicOffsetCount:*/ 0,
        /*pDynamicOffsets*/     nullptr
    );
}

void VKPipelineState::BindDynamicDescriptorSet(VkCommandBuffer commandBuffer, VkDescriptorSet descriptorSet)
{
    if (pipelineLayout_ != nullptr && descriptorSet != VK_NULL_HANDLE)
        BindDescriptorSets(commandBuffer, pipelineLayout_->GetBindPointForDynamicBindings(), 1, &descriptorSet);
}

void VKPipelineState::BindHeapDescriptorSet(VkCommandBuffer commandBuffer, VkDescriptorSet descriptorSet)
{
    if (pipelineLayout_ != nullptr && descriptorSet != VK_NULL_HANDLE)
        BindDescriptorSets(commandBuffer, pipelineLayout_->GetBindPointForHeapBindings(), 1, &descriptorSet);
}

void VKPipelineState::PushConstants(VkCommandBuffer commandBuffer, std::uint32_t first, const char* data, std::uint32_t size)
{
    if (first >= uniformRanges_.size())
        return /*OutOfBounds*/;

    VkPipelineLayout layout = GetVkPipelineLayout();

    const char* pendingData = data;
    VkPushConstantRange pendingRange = {};

    auto FlushPushConstants = [&pendingRange, &pendingData, layout, commandBuffer]()
    {
        if (pendingRange.size > 0)
        {
            vkCmdPushConstants(
                commandBuffer,
                layout,
                pendingRange.stageFlags,
                pendingRange.offset,
                pendingRange.size,
                pendingData
            );
            pendingData += pendingRange.size;
            pendingRange.size = 0;
        }
    };

    for (const std::uint32_t end = static_cast<std::uint32_t>(uniformRanges_.size()); first < end; ++first)
    {
        /* Stop once we reached end of input data */
        const VkPushConstantRange& currentRange = uniformRanges_[first];
        if (size < currentRange.size)
            break;

        if (currentRange.offset > pendingRange.offset + pendingRange.size || currentRange.stageFlags != pendingRange.stageFlags)
        {
            FlushPushConstants();
            pendingRange.stageFlags = currentRange.stageFlags;
            pendingRange.offset     = currentRange.offset;
        }

        pendingRange.size   += currentRange.size;
        size                -= currentRange.size;
    }

    FlushPushConstants();
}


/*
 * ======= Protected: =======
 */

VkPipeline* VKPipelineState::ReleaseAndGetAddressOfVkPipeline()
{
    return pipeline_.ReleaseAndGetAddressOf();
}

VkPipelineLayout VKPipelineState::GetVkPipelineLayout() const
{
    if (pipelineLayoutPerm_.Get() != VK_NULL_HANDLE)
        return pipelineLayoutPerm_.Get();
    if (pipelineLayout_ != nullptr)
        return pipelineLayout_->GetVkPipelineLayout();
    return VKPipelineLayout::GetDefault();
}

void VKPipelineState::GetShaderCreateInfoAndOptionalPermutation(VKShader& shaderVK, VkPipelineShaderStageCreateInfo& outCreateInfo)
{
    shaderVK.FillShaderStageCreateInfo(outCreateInfo);
    if (pipelineLayout_ != nullptr && pipelineLayout_->NeedsShaderModulePermutation(shaderVK))
        outCreateInfo.module = VKShaderModulePool::Get().GetOrCreateVkShaderModulePermutation(shaderVK, *pipelineLayout_);
}


} // /namespace LLGL



// ================================================================================
