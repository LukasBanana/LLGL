/*
 * VKPipelineState.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKPipelineState.h"
#include "VKPipelineLayout.h"
#include "../Shader/VKShader.h"
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
    return nullptr; //TODO
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
    VkPipelineLayout layout = GetVkPipelineLayout();

    for (auto end = static_cast<std::uint32_t>(uniformRanges_.size()); first < end; ++first)
    {
        const auto& pushConstantRange = uniformRanges_[first];
        if (size < pushConstantRange.size)
            return /*OutOfBounds*/;

        vkCmdPushConstants(
            commandBuffer,
            layout,
            pushConstantRange.stageFlags,
            pushConstantRange.offset,
            pushConstantRange.size,
            data
        );

        data += pushConstantRange.size;
    }
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

void VKPipelineState::GetShaderCreateInfoAndOptionalPermutation(
    VKShader&                           shaderVK,
    VkPipelineShaderStageCreateInfo&    outCreateInfo,
    VKPtr<VkShaderModule>&              outShaderModulePermutation)
{
    shaderVK.FillShaderStageCreateInfo(outCreateInfo);
    if (pipelineLayout_ != nullptr)
    {
        outShaderModulePermutation = pipelineLayout_->CreateVkShaderModulePermutation(shaderVK);
        if (outShaderModulePermutation.Get() != VK_NULL_HANDLE)
            outCreateInfo.module = outShaderModulePermutation.Get();
    }
}


} // /namespace LLGL



// ================================================================================
