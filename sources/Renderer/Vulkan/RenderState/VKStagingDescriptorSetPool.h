/*
 * VKStagingDescriptorSetPool.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_VK_STAGING_DESCRIPTOR_SET_POOL_H
#define LLGL_VK_STAGING_DESCRIPTOR_SET_POOL_H


#include "VKStagingDescriptorPool.h"
#include <vector>


namespace LLGL
{


// Pool of Vulkan staging descriptor sets.
class VKStagingDescriptorSetPool
{

    public:

        VKStagingDescriptorSetPool(VkDevice device);

        // Resets all chunks in the pool.
        void Reset();

        // Copies the specified source descriptors into the native D3D descriptor heap.
        VkDescriptorSet AllocateDescriptorSet(
            VkDescriptorSetLayout       setLayout,
            std::uint32_t               numSizes,
            const VkDescriptorPoolSize* sizes
        );

    private:

        // Allocates a new descriptor pool with increased capacity.
        void AllocateDescriptorPool();

    private:

        VkDevice                                device_                 = VK_NULL_HANDLE;
        std::vector<VKStagingDescriptorPool>    descriptorPools_;
        std::size_t                             descriptorPoolIndex_    = 0;
        std::uint32_t                           capacityLevel_          = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
