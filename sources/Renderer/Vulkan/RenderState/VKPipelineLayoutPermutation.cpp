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


VKPipelineLayoutPermutation::VKPipelineLayoutPermutation(
    VkDevice                                device,
    const VKPipelineLayout*                 owner,
    VkDescriptorSetLayout                   setLayoutImmutableSamplers,
    const VKLayoutPermutationParameters&    permutationParams)
:
    owner_                    { owner                                  },
    pipelineLayout_           { device, vkDestroyPipelineLayout        },
    setLayoutHeapBindings_    { device                                 },
    setLayoutDynamicBindings_ { device                                 },
    descriptorPool_           { device, vkDestroyDescriptorPool        },
    pushConstantRanges_       { permutationParams.pushConstantRanges   },
    numImmutableSamplers_     { permutationParams.numImmutableSamplers }
{
    /* Create Vulkan descriptor set layouts */
    if (!permutationParams.setLayoutHeapBindings.empty())
        CreateBindingSetLayout(device, permutationParams.setLayoutHeapBindings, bindingTable_.heapBindings, setLayoutHeapBindings_);
    if (!permutationParams.setLayoutDynamicBindings.empty())
        CreateBindingSetLayout(device, permutationParams.setLayoutDynamicBindings, bindingTable_.dynamicBindings, setLayoutDynamicBindings_);

    /* Create descriptor pool for dynamic descriptors and immutable samplers */
    if (!bindingTable_.dynamicBindings.empty() || numImmutableSamplers_ > 0)
        CreateDescriptorPool(device, numImmutableSamplers_);
    if (!bindingTable_.dynamicBindings.empty())
        CreateDescriptorCache(device, setLayoutDynamicBindings_.GetVkDescriptorSetLayout());

    /* Create native Vulkan pipeline layout */
    pipelineLayout_ = CreateVkPipelineLayout(device, setLayoutImmutableSamplers);
}

static int ComparePushConstantRangeSWO(const VkPushConstantRange& lhs, const VkPushConstantRange& rhs)
{
    LLGL_COMPARE_MEMBER_SWO( stageFlags );
    LLGL_COMPARE_MEMBER_SWO( offset     );
    LLGL_COMPARE_MEMBER_SWO( size       );
    return 0;
}

int VKPipelineLayoutPermutation::CompareSWO(const VKPipelineLayoutPermutation& lhs, const VKLayoutPermutationParameters& rhs)
{
    LLGL_COMPARE_SEPARATE_FUNC_SWO(VKDescriptorSetLayout::CompareSWO, lhs.setLayoutHeapBindings_, rhs.setLayoutHeapBindings);
    LLGL_COMPARE_SEPARATE_FUNC_SWO(VKDescriptorSetLayout::CompareSWO, lhs.setLayoutDynamicBindings_, rhs.setLayoutDynamicBindings);

    LLGL_COMPARE_SEPARATE_MEMBERS_SWO(lhs.pushConstantRanges_.size(), rhs.pushConstantRanges.size());
    LLGL_COMPARE_SEPARATE_MEMBERS_SWO(lhs.numImmutableSamplers_, rhs.numImmutableSamplers);

    for_range(i, lhs.pushConstantRanges_.size())
        LLGL_COMPARE_SEPARATE_FUNC_SWO(ComparePushConstantRangeSWO, lhs.pushConstantRanges_[i], rhs.pushConstantRanges[i]);

    return 0;
}


/*
 * ======= Private: =======
 */

void VKPipelineLayoutPermutation::CreateBindingSetLayout(
    VkDevice                                    device,
    std::vector<VkDescriptorSetLayoutBinding>   setLayoutBindings,
    std::vector<VKLayoutBinding>&               outBindings,
    VKDescriptorSetLayout&                      outSetLayout)
{
    outSetLayout.Initialize(device, std::move(setLayoutBindings));
    outSetLayout.GetLayoutBindings(outBindings);
}

VKPtr<VkPipelineLayout> VKPipelineLayoutPermutation::CreateVkPipelineLayout(VkDevice device, VkDescriptorSetLayout setLayoutImmutableSamplers) const
{
    /* Gather array of up to 3 set layouts */
    SmallVector<VkDescriptorSetLayout, 3> setLayoutsVK;

    if (setLayoutHeapBindings_.GetVkDescriptorSetLayout() != VK_NULL_HANDLE)
        setLayoutsVK.push_back(setLayoutHeapBindings_.GetVkDescriptorSetLayout());
    if (setLayoutDynamicBindings_.GetVkDescriptorSetLayout() != VK_NULL_HANDLE)
        setLayoutsVK.push_back(setLayoutDynamicBindings_.GetVkDescriptorSetLayout());
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
        if (pushConstantRanges_.empty())
        {
            layoutCreateInfo.pushConstantRangeCount = 0;
            layoutCreateInfo.pPushConstantRanges    = nullptr;
        }
        else
        {
            layoutCreateInfo.pushConstantRangeCount = static_cast<std::uint32_t>(pushConstantRanges_.size());
            layoutCreateInfo.pPushConstantRanges    = pushConstantRanges_.data();
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
