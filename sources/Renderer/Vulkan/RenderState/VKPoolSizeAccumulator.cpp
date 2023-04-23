/*
 * VKPoolSizeAccumulator.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "VKPoolSizeAccumulator.h"
#include <LLGL/Utils/ForRange.h>


namespace LLGL
{


// Returns the zero-based index of a descriptor pool for the specified descriptor type
static std::uint32_t GetPoolIndex(VkDescriptorType type)
{
    if (type >= VK_DESCRIPTOR_TYPE_SAMPLER && type <= VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT)
        return static_cast<std::uint32_t>(type);
    else
        return -1;
}

void VKPoolSizeAccumulator::Accumulate(VkDescriptorType type, std::uint32_t count)
{
    const auto poolIndex = GetPoolIndex(type);
    countsPerType_[poolIndex] += count;
}

void VKPoolSizeAccumulator::Finalize()
{
    for_range(i, numDescriptorTypes)
    {
        if (countsPerType_[i] > 0)
        {
            VkDescriptorPoolSize poolSizeInfo;
            {
                poolSizeInfo.type               = static_cast<VkDescriptorType>(i);
                poolSizeInfo.descriptorCount    = countsPerType_[i];
            }
            poolSizes_.push_back(poolSizeInfo);
        }
    }
}


} // /namespace LLGL



// ================================================================================
