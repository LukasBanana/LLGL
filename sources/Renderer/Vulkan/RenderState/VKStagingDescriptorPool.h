/*
 * VKStagingDescriptorPool.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_VK_STAGING_DESCRIPTOR_POOL_H
#define LLGL_VK_STAGING_DESCRIPTOR_POOL_H


#include "../Vulkan.h"
#include "../VKPtr.h"
#include <vector>
#include <cstdint>


namespace LLGL
{


// Pool of Vulkan staging descriptor sets.
class VKStagingDescriptorPool
{

    public:

        // Number of descriptor types.
        static constexpr int numDescriptorTypes = (static_cast<int>(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT) + 1);

    public:

        VKStagingDescriptorPool(VkDevice device);

        VKStagingDescriptorPool(const VKStagingDescriptorPool&) = delete;
        VKStagingDescriptorPool& operator = (const VKStagingDescriptorPool&) = delete;

        VKStagingDescriptorPool(VKStagingDescriptorPool&& rhs);
        VKStagingDescriptorPool& operator = (VKStagingDescriptorPool&& rhs);

        // Allocates the native Vulkan descriptor pool.
        void Initialize(
            std::uint32_t               setCapacity,
            std::uint32_t               numPoolSizes,
            const VkDescriptorPoolSize* poolSizes
        );

        // Resets all previously allocated descriptor sets and frees all memory of this descriptor pool.
        void Reset();

        // Returns true if this pool can allocate another descriptor set with the specified sizes.
        bool Capacity(std::uint32_t numSizes, const VkDescriptorPoolSize* sizes) const;

        // Allocates a new descriptor set with the specified set layout.
        // Traps execution if the pool ran out of its capacity. Use Capacity() to check its capacity first.
        VkDescriptorSet AllocateDescriptorSet(
            VkDescriptorSetLayout       setLayout,
            std::uint32_t               numSizes,
            const VkDescriptorPoolSize* sizes
        );

    private:

        VkDevice                device_                             = VK_NULL_HANDLE;
        VKPtr<VkDescriptorPool> descriptorPool_;
        std::uint32_t           poolCapacities_[numDescriptorTypes] = {};
        std::uint32_t           poolSizes_[numDescriptorTypes]      = {};
        std::uint32_t           setCapacity_                        = 0;
        std::uint32_t           setSize_                            = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
