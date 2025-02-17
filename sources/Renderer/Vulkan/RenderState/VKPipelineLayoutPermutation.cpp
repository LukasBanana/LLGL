/*
 * VKPipelineLayoutPermutation.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "VKPipelineLayoutPermutation.h"
#include "VKPoolSizeAccumulator.h"
#include "../VKTypes.h"
#include "../VKCore.h"
#include "../VKStaticLimits.h"
#include "../Texture/VKSampler.h"
#include "../Shader/VKShader.h"
#include "../Shader/VKShaderModulePool.h"
#include "../../ResourceUtils.h"
#include "../../../Core/CoreUtils.h"
#include "../../../Core/MacroUtils.h"
#include "../../../Core/Assertion.h"
#include <LLGL/Utils/ForRange.h>
#include <LLGL/Container/SmallVector.h>
#include <algorithm>


namespace LLGL
{


static int CompareSetLayoutBindingSWO(const VkDescriptorSetLayoutBinding& lhs, const VkDescriptorSetLayoutBinding& rhs)
{
    LLGL_COMPARE_MEMBER_SWO( binding            );
    LLGL_COMPARE_MEMBER_SWO( descriptorType     );
    LLGL_COMPARE_MEMBER_SWO( descriptorCount    );
    LLGL_COMPARE_MEMBER_SWO( stageFlags         );
  //LLGL_COMPARE_MEMBER_SWO( pImmutableSamplers );
    return 0;
}

static int ComparePushConstantRangeSWO(const VkPushConstantRange& lhs, const VkPushConstantRange& rhs)
{
    LLGL_COMPARE_MEMBER_SWO( stageFlags );
    LLGL_COMPARE_MEMBER_SWO( offset     );
    LLGL_COMPARE_MEMBER_SWO( size       );
    return 0;
}

int VKLayoutPermutationParameters::CompareSWO(const VKLayoutPermutationParameters& lhs, const VKLayoutPermutationParameters& rhs)
{
    LLGL_COMPARE_SEPARATE_MEMBERS_SWO(lhs.setLayoutHeapBindings.size(), rhs.setLayoutHeapBindings.size());
    LLGL_COMPARE_SEPARATE_MEMBERS_SWO(lhs.setLayoutDynamicBindings.size(), rhs.setLayoutDynamicBindings.size());
    LLGL_COMPARE_SEPARATE_MEMBERS_SWO(lhs.pushConstantRanges.size(), rhs.pushConstantRanges.size());
    LLGL_COMPARE_MEMBER_SWO(numImmutableSamplers);

    for_range(i, lhs.setLayoutHeapBindings.size())
    {
        int cmp = CompareSetLayoutBindingSWO(lhs.setLayoutHeapBindings[i], rhs.setLayoutHeapBindings[i]);
        if (cmp != 0)
            return cmp;
    }

    for_range(i, lhs.setLayoutDynamicBindings.size())
    {
        int cmp = CompareSetLayoutBindingSWO(lhs.setLayoutDynamicBindings[i], rhs.setLayoutDynamicBindings[i]);
        if (cmp != 0)
            return cmp;
    }

    for_range(i, lhs.pushConstantRanges.size())
    {
        int cmp = ComparePushConstantRangeSWO(lhs.pushConstantRanges[i], rhs.pushConstantRanges[i]);
        if (cmp != 0)
            return cmp;
    }

    return 0;
}

VKPipelineLayoutPermutation::VKPipelineLayoutPermutation(
    VkDevice                                device,
    const VKPipelineLayout*                 owner,
    VkDescriptorSetLayout                   setLayoutImmutableSamplers,
    const VKLayoutPermutationParameters&    permutationParams)
:
    owner_                    { owner                                },
    pipelineLayout_           { device, vkDestroyPipelineLayout      },
    setLayoutHeapBindings_    { device, vkDestroyDescriptorSetLayout },
    setLayoutDynamicBindings_ { device, vkDestroyDescriptorSetLayout },
    descriptorPool_           { device, vkDestroyDescriptorPool      }
{
    /* Create Vulkan descriptor set layouts */
    if (!permutationParams.setLayoutHeapBindings.empty())
        CreateBindingSetLayout(device, permutationParams.setLayoutHeapBindings, bindingTable_.heapBindings, setLayoutHeapBindings_);
    if (!permutationParams.setLayoutDynamicBindings.empty())
        CreateBindingSetLayout(device, permutationParams.setLayoutDynamicBindings, bindingTable_.dynamicBindings, setLayoutDynamicBindings_);

    /* Create descriptor pool for dynamic descriptors and immutable samplers */
    if (!bindingTable_.dynamicBindings.empty() || permutationParams.numImmutableSamplers > 0)
        CreateDescriptorPool(device, permutationParams.numImmutableSamplers);
    if (!bindingTable_.dynamicBindings.empty())
        CreateDescriptorCache(device, setLayoutDynamicBindings_.Get());

    /* Create native Vulkan pipeline layout */
    pipelineLayout_ = CreateVkPipelineLayout(device, setLayoutImmutableSamplers, permutationParams.pushConstantRanges);
}


/*
 * ======= Private: =======
 */

void VKPipelineLayoutPermutation::CreateVkDescriptorSetLayout(
    VkDevice                                        device,
    const ArrayView<VkDescriptorSetLayoutBinding>&  setLayoutBindings,
    VKPtr<VkDescriptorSetLayout>&                   outSetLayout)
{
    VkDescriptorSetLayoutCreateInfo createInfo;
    {
        createInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        createInfo.pNext        = nullptr;
        createInfo.flags        = 0;
        createInfo.bindingCount = static_cast<std::uint32_t>(setLayoutBindings.size());
        createInfo.pBindings    = setLayoutBindings.data();
    }
    VkResult result = vkCreateDescriptorSetLayout(device, &createInfo, nullptr, outSetLayout.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Vulkan descriptor set layout");
}

void VKPipelineLayoutPermutation::CreateBindingSetLayout(
    VkDevice                                            device,
    const std::vector<VkDescriptorSetLayoutBinding>&    setLayoutBindings,
    std::vector<VKLayoutBinding>&                       outBindings,
    VKPtr<VkDescriptorSetLayout>&                       outSetLayout)
{
    const std::size_t numBindings = setLayoutBindings.size();
    CreateVkDescriptorSetLayout(device, setLayoutBindings, outSetLayout);

    /* Create list of binding points (for later pass to 'VkWriteDescriptorSet::dstBinding') */
    outBindings.reserve(numBindings);
    for_range(i, numBindings)
    {
        for_range(arrayElement, setLayoutBindings[i].descriptorCount)
        {
            outBindings.push_back(
                VKLayoutBinding
                {
                    /*dstBinding:*/         setLayoutBindings[i].binding,
                    /*dstArrayElement:*/    arrayElement,
                    /*descriptorType:*/     setLayoutBindings[i].descriptorType,
                    /*stageFlags:*/         setLayoutBindings[i].stageFlags
                }
            );
        }
    }
}

VKPtr<VkPipelineLayout> VKPipelineLayoutPermutation::CreateVkPipelineLayout(
    VkDevice                                device,
    VkDescriptorSetLayout                   setLayoutImmutableSamplers,
    const ArrayView<VkPushConstantRange>&   pushConstantRanges) const
{
    /* Gather array of up to 3 set layouts */
    SmallVector<VkDescriptorSetLayout, 3> setLayoutsVK;

    if (setLayoutHeapBindings_.Get() != VK_NULL_HANDLE)
        setLayoutsVK.push_back(setLayoutHeapBindings_.Get());
    if (setLayoutDynamicBindings_.Get() != VK_NULL_HANDLE)
        setLayoutsVK.push_back(setLayoutDynamicBindings_.Get());
    if (setLayoutImmutableSamplers != VK_NULL_HANDLE)
        setLayoutsVK.push_back(setLayoutImmutableSamplers);

    /* Create native Vulkan pipeline layout */
    VkPipelineLayoutCreateInfo layoutCreateInfo;
    {
        layoutCreateInfo.sType                      = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        layoutCreateInfo.pNext                      = nullptr;
        layoutCreateInfo.flags                      = 0;
        layoutCreateInfo.setLayoutCount             = static_cast<std::uint32_t>(setLayoutsVK.size());
        layoutCreateInfo.pSetLayouts                = setLayoutsVK.data();
        if (pushConstantRanges.empty())
        {
            layoutCreateInfo.pushConstantRangeCount = 0;
            layoutCreateInfo.pPushConstantRanges    = nullptr;
        }
        else
        {
            layoutCreateInfo.pushConstantRangeCount = static_cast<std::uint32_t>(pushConstantRanges.size());
            layoutCreateInfo.pPushConstantRanges    = pushConstantRanges.data();
        }
    }
    VKPtr<VkPipelineLayout> pipelineLayout{ device, vkDestroyPipelineLayout };
    VkResult result = vkCreatePipelineLayout(device, &layoutCreateInfo, nullptr, pipelineLayout.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Vulkan pipeline layout");
    return pipelineLayout;
}

void VKPipelineLayoutPermutation::CreateDescriptorPool(VkDevice device, std::uint32_t numImmutableSamplers)
{
    /* Accumulate descriptor pool sizes for all dynamic resources and immutable samplers */
    VKPoolSizeAccumulator poolSizeAccum;

    for (const VKLayoutBinding& binding : bindingTable_.dynamicBindings)
        poolSizeAccum.Accumulate(binding.descriptorType);

    if (numImmutableSamplers > 0)
        poolSizeAccum.Accumulate(VK_DESCRIPTOR_TYPE_SAMPLER, numImmutableSamplers);

    poolSizeAccum.Finalize();

    /* Create Vulkan descriptor pool */
    VkDescriptorPoolCreateInfo poolCreateInfo;
    {
        poolCreateInfo.sType            = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolCreateInfo.pNext            = nullptr;
        poolCreateInfo.flags            = 0;
        poolCreateInfo.maxSets          = 2;
        poolCreateInfo.poolSizeCount    = poolSizeAccum.Size();
        poolCreateInfo.pPoolSizes       = poolSizeAccum.Data();
    }
    VkResult result = vkCreateDescriptorPool(device, &poolCreateInfo, nullptr, descriptorPool_.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Vulkan descriptor pool for static samplers");
}

void VKPipelineLayoutPermutation::CreateDescriptorCache(VkDevice device, VkDescriptorSetLayout setLayout)
{
    /*
    Don't account descriptors in the dynamic cache for immutable samplers,
    so accumulate pool sizes only for dynamic resources here.
    */
    VKPoolSizeAccumulator poolSizeAccum;
    for (const VKLayoutBinding& binding : bindingTable_.dynamicBindings)
        poolSizeAccum.Accumulate(binding.descriptorType);
    poolSizeAccum.Finalize();

    /* Allocate unique descriptor cache */
    descriptorCache_ = MakeUnique<VKDescriptorCache>(
        device, descriptorPool_, setLayout, poolSizeAccum.Size(), poolSizeAccum.Data(), bindingTable_.dynamicBindings
    );
}


} // /namespace LLGL



// ================================================================================
