/*
 * VKPoolSizeAccumulator.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_VK_POOL_SIZE_ACCUMULATOR_H
#define LLGL_VK_POOL_SIZE_ACCUMULATOR_H


#include <vulkan/vulkan.h>
#include <LLGL/Container/SmallVector.h>
#include <cstdint>


namespace LLGL
{


// Helper structure to handle buffer and image information for a descriptor set.
class VKPoolSizeAccumulator
{

    public:

        static constexpr auto numDescriptorTypes = (static_cast<std::uint32_t>(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT) + 1);

    public:

        // Accumulates the specified count for the type of descriptors to this container.
        void Accumulate(VkDescriptorType type, std::uint32_t count = 1);

        // Finalizes the container.
        void Finalize();

        // Returns the size of this container.
        inline std::uint32_t Size() const
        {
            return static_cast<std::uint32_t>(poolSizes_.size());
        }

        // Returns the data of this container.
        inline const VkDescriptorPoolSize* Data() const
        {
            return poolSizes_.data();
        }

    private:

        SmallVector<VkDescriptorPoolSize, numDescriptorTypes>   poolSizes_;
        std::uint32_t                                           countsPerType_[numDescriptorTypes] = {};

};


} // /namespace LLGL


#endif



// ================================================================================
