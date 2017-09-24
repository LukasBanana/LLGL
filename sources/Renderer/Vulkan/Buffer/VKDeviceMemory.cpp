/*
 * VKDeviceMemory.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKDeviceMemory.h"
#include "../VKCore.h"


namespace LLGL
{


VKDeviceMemory::VKDeviceMemory(const VKPtr<VkDevice>& device, VkDeviceSize size, uint32_t memoryTypeIndex) :
    deviceMemory_ { device, vkFreeMemory }
{
    /* Allocate device memory */
    VkMemoryAllocateInfo allocInfo;
    {
        allocInfo.sType             = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.pNext             = nullptr;
        allocInfo.allocationSize    = size;
        allocInfo.memoryTypeIndex   = memoryTypeIndex;
    }
    auto result = vkAllocateMemory(device, &allocInfo, nullptr, deviceMemory_.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to allocate Vulkan device memory of " + std::to_string(size) + " bytes");
}


} // /namespace LLGL



// ================================================================================
