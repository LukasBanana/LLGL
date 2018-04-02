/*
 * VKResourceViewHeap.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKResourceViewHeap.h"
#include "VKPipelineLayout.h"
#include "../../CheckedCast.h"
#include "../VKTypes.h"
#include "../VKCore.h"


namespace LLGL
{


VKResourceViewHeap::VKResourceViewHeap(const VKPtr<VkDevice>& device, const ResourceViewHeapDescriptor& desc) :
    device_ { device }
{
    /* Get pipeline layout object */
    /* Get shader program object */
    auto pipelineLayoutVK = LLGL_CAST(VKPipelineLayout*, desc.pipelineLayout);
    if (!pipelineLayoutVK)
        throw std::invalid_argument("failed to create resource view heap due to missing pipeline layout");

    //pipelineLayout_ = pipelineLayoutVK->Get();

    /* Initialize descriptor pool sizes */
    std::vector<VkDescriptorPoolSize> poolSizes(desc.resourceViews.size());
    for (std::size_t i = 0; i < desc.resourceViews.size(); ++i)
    {
        poolSizes[i].type            = VKTypes::Map(desc.resourceViews[i].type);
        poolSizes[i].descriptorCount = 1;
    }

    /* Create descriptor pool */
    VkDescriptorPoolCreateInfo poolCreateInfo;
    {
        poolCreateInfo.sType            = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolCreateInfo.pNext            = nullptr;
        poolCreateInfo.flags            = 0;
        poolCreateInfo.maxSets          = 1;
        poolCreateInfo.poolSizeCount    = static_cast<std::uint32_t>(poolSizes.size());
        poolCreateInfo.pPoolSizes       = poolSizes.data();
    }
    auto result = vkCreateDescriptorPool(device, &poolCreateInfo, nullptr, descriptorPool_.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Vulkan descriptor pool");

    /* Allocate descriptor set */
    VkDescriptorSetLayout setLayouts[] = { pipelineLayoutVK->GetDescriptorSetLayout() };

    VkDescriptorSetAllocateInfo allocInfo;
    {
        allocInfo.sType                 = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.pNext                 = nullptr;
        allocInfo.descriptorPool        = descriptorPool_;
        allocInfo.descriptorSetCount    = 1;
        allocInfo.pSetLayouts           = setLayouts;
    }
    result = vkAllocateDescriptorSets(device, &allocInfo, &descriptorSet_);
    VKThrowIfFailed(result, "failed to allocate Vulkan descriptor sets");
}

VKResourceViewHeap::~VKResourceViewHeap()
{
    // todo
}


} // /namespace LLGL



// ================================================================================
