/*
 * VKStagingDescriptorPool.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "VKStagingDescriptorPool.h"
#include "../VKCore.h"
#include "../../../Core/Assertion.h"
#include <LLGL/Utils/ForRange.h>
#include <string.h>


namespace LLGL
{


VKStagingDescriptorPool::VKStagingDescriptorPool(VkDevice device) :
    device_         { device                          },
    descriptorPool_ { device, vkDestroyDescriptorPool }
{
}

VKStagingDescriptorPool::VKStagingDescriptorPool(VKStagingDescriptorPool&& rhs) :
    device_         { rhs.device_                    },
    descriptorPool_ { std::move(rhs.descriptorPool_) },
    setCapacity_    { rhs.setCapacity_               },
    setSize_        { rhs.setSize_                   }
{
    ::memcpy(poolCapacities_, rhs.poolCapacities_, sizeof(poolCapacities_));
    ::memcpy(poolSizes_, rhs.poolSizes_, sizeof(poolSizes_));
}

VKStagingDescriptorPool& VKStagingDescriptorPool::operator = (VKStagingDescriptorPool&& rhs)
{
    device_         = rhs.device_;
    descriptorPool_ = std::move(rhs.descriptorPool_);
    setCapacity_    = rhs.setCapacity_;
    setSize_        = rhs.setSize_;
    ::memcpy(poolCapacities_, rhs.poolCapacities_, sizeof(poolCapacities_));
    ::memcpy(poolSizes_, rhs.poolSizes_, sizeof(poolSizes_));
    return *this;
}

void VKStagingDescriptorPool::Initialize(
    std::uint32_t               setCapacity,
    std::uint32_t               numPoolSizes,
    const VkDescriptorPoolSize* poolSizes)
{
    /* Store pool configurations */
    setCapacity_ = setCapacity;

    ::memset(poolCapacities_, 0, sizeof(poolCapacities_));
    ::memset(poolSizes_, 0, sizeof(poolSizes_));

    for_range(i, numPoolSizes)
    {
        const auto& poolSize = poolSizes[i];
        poolCapacities_[static_cast<int>(poolSize.type)] = poolSize.descriptorCount;
    }

    /* Create native Vulkan descriptor pool */
    VkDescriptorPoolCreateInfo poolCreateInfo;
    {
        poolCreateInfo.sType            = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolCreateInfo.pNext            = nullptr;
        poolCreateInfo.flags            = 0;
        poolCreateInfo.maxSets          = setCapacity_;
        poolCreateInfo.poolSizeCount    = numPoolSizes;
        poolCreateInfo.pPoolSizes       = poolSizes;
    }
    VkResult result = vkCreateDescriptorPool(device_, &poolCreateInfo, nullptr, descriptorPool_.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Vulkan descriptor pool");
}

void VKStagingDescriptorPool::Reset()
{
    if (setSize_ > 0)
    {
        /* Reset the Vulkan descriptor pool and all recorded pool sizes */
        vkResetDescriptorPool(device_, descriptorPool_, 0);
        ::memset(poolSizes_, 0, sizeof(poolSizes_));
        setSize_ = 0;
    }
}

bool VKStagingDescriptorPool::Capacity(std::uint32_t numSizes, const VkDescriptorPoolSize* sizes) const
{
    if (setSize_ == setCapacity_)
        return false;
    for_range(i, numSizes)
    {
        const int typeIndex = static_cast<int>(sizes[i].type);
        if (poolSizes_[typeIndex] + sizes[i].descriptorCount > poolCapacities_[typeIndex])
            return false;
    }
    return true;
}

VkDescriptorSet VKStagingDescriptorPool::AllocateDescriptorSet(
    VkDescriptorSetLayout       setLayout,
    std::uint32_t               numSizes,
    const VkDescriptorPoolSize* sizes)
{
    LLGL_ASSERT(setSize_ < setCapacity_);

    /* Increase pool sizes */
    ++setSize_;
    for_range(i, numSizes)
        poolSizes_[static_cast<int>(sizes[i].type)] += sizes[i].descriptorCount;

    /* Allocate single descriptor set */
    VkDescriptorSetAllocateInfo allocInfo;
    {
        allocInfo.sType                 = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.pNext                 = nullptr;
        allocInfo.descriptorPool        = descriptorPool_;
        allocInfo.descriptorSetCount    = 1;
        allocInfo.pSetLayouts           = &setLayout;
    }
    VkDescriptorSet descriptorSet;
    VkResult result = vkAllocateDescriptorSets(device_, &allocInfo, &descriptorSet);
    VKThrowIfFailed(result, "failed to allocate Vulkan descriptor sets");
    return descriptorSet;
}


} // /namespace LLGL



// ================================================================================
