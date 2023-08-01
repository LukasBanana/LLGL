/*
 * VKDeviceMemoryRegion.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "VKDeviceMemoryRegion.h"
#include "VKDeviceMemory.h"
#include "../VKCore.h"


namespace LLGL
{


VKDeviceMemoryRegion::VKDeviceMemoryRegion(VKDeviceMemory* deviceMemory, VkDeviceSize alignedSize, VkDeviceSize alignedOffset, std::uint32_t memoryTypeIndex) :
    deviceMemory_    { deviceMemory    },
    size_            { alignedSize     },
    offset_          { alignedOffset   },
    memoryTypeIndex_ { memoryTypeIndex }
{
}

void VKDeviceMemoryRegion::BindBuffer(VkDevice device, VkBuffer buffer)
{
    vkBindBufferMemory(device, buffer, deviceMemory_->GetVkDeviceMemory(), GetOffset());
}

void VKDeviceMemoryRegion::BindImage(VkDevice device, VkImage image)
{
    vkBindImageMemory(device, image, deviceMemory_->GetVkDeviceMemory(), GetOffset());
}


/*
 * ======= Protected: =======
 */

bool VKDeviceMemoryRegion::MergeWith(VKDeviceMemoryRegion& other)
{
    if (GetParentChunk() == other.GetParentChunk() && GetMemoryTypeIndex() == other.GetMemoryTypeIndex())
    {
        /* Check if this region is a direct lower part */
        if (GetOffsetWithSize() == other.GetOffset())
        {
            size_ += other.GetSize();
            return true;
        }

        /* Check if this region is a direct upper part */
        if (other.GetOffsetWithSize() == GetOffset())
        {
            offset_ = other.GetOffset();
            size_ += other.GetSize();
            return true;
        }
    }
    return false;
}

void VKDeviceMemoryRegion::MoveAt(VkDeviceSize alignedSize, VkDeviceSize alignedOffset)
{
    size_   = alignedSize;
    offset_ = alignedOffset;
}


} // /namespace LLGL



// ================================================================================
