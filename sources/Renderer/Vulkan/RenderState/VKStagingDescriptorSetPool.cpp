/*
 * VKStagingDescriptorSetPool.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "VKStagingDescriptorSetPool.h"
#include <LLGL/Utils/ForRange.h>
#include <algorithm>


namespace LLGL
{


VKStagingDescriptorSetPool::VKStagingDescriptorSetPool(VkDevice device) :
    device_ { device }
{
}

void VKStagingDescriptorSetPool::Reset()
{
    if (!descriptorPools_.empty())
    {
        for_range(i, descriptorPoolIndex_ + 1)
            descriptorPools_[i].Reset();
        descriptorPoolIndex_ = 0;
    }
}

VkDescriptorSet VKStagingDescriptorSetPool::AllocateDescriptorSet(
    VkDescriptorSetLayout       setLayout,
    std::uint32_t               numSizes,
    const VkDescriptorPoolSize* sizes)
{
    if (descriptorPools_.empty())
    {
        /* Allocate initial descriptor pool */
        AllocateDescriptorPool();
    }
    else if (!descriptorPools_[descriptorPoolIndex_].Capacity(numSizes, sizes))
    {
        /* Move to next descriptor pool and allocate new one as needed */
        ++descriptorPoolIndex_;
        if (descriptorPoolIndex_ == descriptorPools_.size())
            AllocateDescriptorPool();
    }
    return descriptorPools_[descriptorPoolIndex_].AllocateDescriptorSet(setLayout, numSizes, sizes);
}


/*
 * ======= Private: =======
 */

static std::uint32_t GetDescriptorSetCapacity(std::uint32_t level)
{
    constexpr std::uint32_t initialCapacity = 256;
    return (initialCapacity << std::min(level, 5u));
}

static std::uint32_t GetDescriptorPoolCapacity(std::uint32_t level)
{
    constexpr std::uint32_t initialCapacity = 1024;
    return (initialCapacity << std::min(level, 5u));
}

void VKStagingDescriptorSetPool::AllocateDescriptorPool()
{
    const std::uint32_t descriptorPoolSize = GetDescriptorPoolCapacity(capacityLevel_);
    const VkDescriptorPoolSize poolSizes[] =
    {
        VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_SAMPLER,              descriptorPoolSize },
        VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,        descriptorPoolSize },
        VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,        descriptorPoolSize },
        VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,       descriptorPoolSize },
        VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,       descriptorPoolSize },
        VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, descriptorPoolSize },
        VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, descriptorPoolSize },
    };
    const std::uint32_t setCapacity = GetDescriptorSetCapacity(capacityLevel_);
    descriptorPools_.emplace_back(device_);
    descriptorPools_.back().Initialize(setCapacity, sizeof(poolSizes)/sizeof(poolSizes[0]), poolSizes);
    ++capacityLevel_;
}



} // /namespace LLGL



// ================================================================================
