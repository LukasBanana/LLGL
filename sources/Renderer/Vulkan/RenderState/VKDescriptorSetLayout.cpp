/*
 * VKDescriptorSetLayout.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "VKDescriptorSetLayout.h"
#include "VKSanitizeBindingSlotContext.h"
#include "../VKCore.h"
#include "../../../Core/Assertion.h"
#include <LLGL/Utils/ForRange.h>
#include <LLGL/Constants.h>
#include <algorithm>
#include <cstring>


namespace LLGL
{


VKDescriptorSetLayout::VKDescriptorSetLayout(VkDevice device) :
    setLayout_ { device, vkDestroyDescriptorSetLayout }
{
}

VKDescriptorSetLayout::VKDescriptorSetLayout(VKDescriptorSetLayout&& rhs) noexcept :
    setLayout_         { std::move(rhs.setLayout_)         },
    setLayoutBindings_ { std::move(rhs.setLayoutBindings_) }
{
}

static VkPipelineStageFlags ShaderStageToPipelineStage(VkShaderStageFlags flags)
{
    VkPipelineStageFlags bitmask = 0;

    if ((flags & VK_SHADER_STAGE_VERTEX_BIT                 ) != 0) { bitmask |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;                  }
    if ((flags & VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT   ) != 0) { bitmask |= VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT;    }
    if ((flags & VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT) != 0) { bitmask |= VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT; }
    if ((flags & VK_SHADER_STAGE_GEOMETRY_BIT               ) != 0) { bitmask |= VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;                }
    if ((flags & VK_SHADER_STAGE_FRAGMENT_BIT               ) != 0) { bitmask |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;                }
    if ((flags & VK_SHADER_STAGE_COMPUTE_BIT                ) != 0) { bitmask |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;                 }

    return bitmask;
}

void VKDescriptorSetLayout::GetLayoutBindings(std::vector<VKLayoutBinding>& outBindings) const
{
    /* Create list of binding points (for later pass to 'VkWriteDescriptorSet::dstBinding') */
    const std::size_t numBindings = setLayoutBindings_.size();
    outBindings.reserve(numBindings);
    for_range(i, numBindings)
    {
        for_range(arrayElement, setLayoutBindings_[i].descriptorCount)
        {
            outBindings.push_back(
                VKLayoutBinding
                {
                    /*dstBinding:*/         setLayoutBindings_[i].binding,
                    /*dstArrayElement:*/    arrayElement,
                    /*barrierSlot:*/        ~0u,
                    /*descriptorType:*/     setLayoutBindings_[i].descriptorType,
                    /*stageFlags:*/         ShaderStageToPipelineStage(setLayoutBindings_[i].stageFlags)
                }
            );
        }
    }
}

void VKDescriptorSetLayout::Initialize(VkDevice device, std::vector<VkDescriptorSetLayoutBinding>&& setLayoutBindings, VKSanitizeBindingSlotContext& sanitizeContext)
{
    setLayoutBindings_ = std::move(setLayoutBindings);
    sanitizeContext.SanitizeBindingSlots(setLayoutBindings_);
    CreateVkDescriptorSetLayout(device);
}

void VKDescriptorSetLayout::UpdateLayoutBindingType(std::uint32_t descriptorIndex, VkDescriptorType descriptorType)
{
    LLGL_ASSERT(descriptorIndex < setLayoutBindings_.size());
    if (setLayoutBindings_[descriptorIndex].descriptorType != descriptorType)
    {
        setLayoutBindings_[descriptorIndex].descriptorType = descriptorType;
        isAnyDescriptorTypeDirty_ = true;
    }
}

void VKDescriptorSetLayout::FinalizeUpdateLayoutBindingTypes(VkDevice device)
{
    if (isAnyDescriptorTypeDirty_)
    {
        CreateVkDescriptorSetLayout(device);
        isAnyDescriptorTypeDirty_ = false;
    }
}

void VKDescriptorSetLayout::CreateVkDescriptorSetLayout(
    VkDevice                                        device,
    const ArrayView<VkDescriptorSetLayoutBinding>&  setLayoutBindings,
    VKPtr<VkDescriptorSetLayout>&                   outDescriptorSetLayout)
{
    VkDescriptorSetLayoutCreateInfo createInfo;
    {
        createInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        createInfo.pNext        = nullptr;
        createInfo.flags        = 0;
        createInfo.bindingCount = static_cast<std::uint32_t>(setLayoutBindings.size());
        createInfo.pBindings    = setLayoutBindings.data();
    }
    VkResult result = vkCreateDescriptorSetLayout(device, &createInfo, nullptr, outDescriptorSetLayout.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Vulkan descriptor set layout");
}

static int CompareSetLayoutBindingSWO(const VkDescriptorSetLayoutBinding& lhs, const VkDescriptorSetLayoutBinding& rhs)
{
    LLGL_COMPARE_MEMBER_SWO( binding            );
    LLGL_COMPARE_MEMBER_SWO( descriptorType     );
    LLGL_COMPARE_MEMBER_SWO( descriptorCount    );
    LLGL_COMPARE_MEMBER_SWO( stageFlags         );
  //LLGL_COMPARE_MEMBER_SWO( pImmutableSamplers );
    return 0;
}

int VKDescriptorSetLayout::CompareSWO(const VKDescriptorSetLayout& lhs, const VKDescriptorSetLayout& rhs)
{
    return VKDescriptorSetLayout::CompareSWO(lhs, rhs.setLayoutBindings_);
}

int VKDescriptorSetLayout::CompareSWO(const VKDescriptorSetLayout& lhs, const std::vector<VkDescriptorSetLayoutBinding>& rhs)
{
    LLGL_COMPARE_SEPARATE_MEMBERS_SWO(lhs.setLayoutBindings_.size(), rhs.size());

    for_range(i, lhs.setLayoutBindings_.size())
    {
        int cmp = CompareSetLayoutBindingSWO(lhs.setLayoutBindings_[i], rhs[i]);
        if (cmp != 0)
            return cmp;
    }

    return 0;
}


/*
 * ======= Private: ======
 */

void VKDescriptorSetLayout::CreateVkDescriptorSetLayout(VkDevice device)
{
    VKDescriptorSetLayout::CreateVkDescriptorSetLayout(device, setLayoutBindings_, setLayout_);
}


} // /namespace LLGL



// ================================================================================
