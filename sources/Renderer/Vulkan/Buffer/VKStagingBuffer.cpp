/*
 * VKStagingBuffer.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "VKStagingBuffer.h"
#include "../Memory/VKDeviceMemoryManager.h"
#include "../../../Core/CoreUtils.h"
#include <string.h>


namespace LLGL
{


VKStagingBuffer::VKStagingBuffer(
    VKDeviceMemoryManager&  deviceMemoryMngr,
    VkDeviceSize            size,
    VkDeviceSize            alignment,
    VkMemoryPropertyFlags   memoryPropertyFlags)
:
    bufferObj_ { deviceMemoryMngr.GetVkDevice() }
{
    Create(deviceMemoryMngr, size, alignment, memoryPropertyFlags);
}

VKStagingBuffer::VKStagingBuffer(VKStagingBuffer&& rhs) noexcept :
    bufferObj_ { std::move(rhs.bufferObj_) },
    size_      { rhs.size_                 },
    offset_    { rhs.offset_               }
{
}

VKStagingBuffer& VKStagingBuffer::operator = (VKStagingBuffer&& rhs) noexcept
{
    if (this != &rhs)
    {
        bufferObj_  = std::move(rhs.bufferObj_);
        size_       = rhs.size_;
        offset_     = rhs.offset_;
    }
    return *this;
}

void VKStagingBuffer::Create(
    VKDeviceMemoryManager&  deviceMemoryMngr,
    VkDeviceSize            size,
    VkDeviceSize            alignment,
    VkMemoryPropertyFlags   memoryPropertyFlags)
{
    size = GetAlignedSize<VkDeviceSize>(size, alignment);

    VkBufferCreateInfo createInfo;
    {
        createInfo.sType                    = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        createInfo.pNext                    = nullptr;
        createInfo.flags                    = 0;
        createInfo.size                     = size;
        createInfo.usage                    = VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
        createInfo.sharingMode              = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount    = 0;
        createInfo.pQueueFamilyIndices      = nullptr;
    }
    bufferObj_.CreateVkBufferAndMemoryRegion(deviceMemoryMngr.GetVkDevice(), createInfo, deviceMemoryMngr, memoryPropertyFlags);

    /* Store new size and reset write offset */
    size_   = size;
    offset_ = 0;
}

void VKStagingBuffer::Reset()
{
    offset_ = 0;
}

bool VKStagingBuffer::Capacity(VkDeviceSize dataSize) const
{
    return (offset_ + dataSize <= size_);
}

VkResult VKStagingBuffer::Write(
    VkDevice        device,
    VkCommandBuffer commandBuffer,
    VkBuffer        dstBuffer,
    VkDeviceSize    dstOffset,
    const void*     data,
    VkDeviceSize    dataSize)
{
    VKDeviceMemoryRegion* region = bufferObj_.GetMemoryRegion();
    if (region == nullptr)
        return VK_ERROR_INITIALIZATION_FAILED;

    /* Map buffer memory to host memory */
    VKDeviceMemory* deviceMemory = region->GetParentChunk();
    void* memory = deviceMemory->Map(device, region->GetOffset() + offset_, dataSize);
    if (memory == nullptr)
        return VK_ERROR_MEMORY_MAP_FAILED;

    /* Copy input data to buffer memory */
    ::memcpy(memory, data, static_cast<std::size_t>(dataSize));
    deviceMemory->Unmap(device);

    /* Encode copy command to transfer staged memory region into destination buffer */
    VkBufferCopy bufferCopy;
    {
        bufferCopy.srcOffset    = offset_;
        bufferCopy.dstOffset    = dstOffset;
        bufferCopy.size         = dataSize;
    }
    vkCmdCopyBuffer(commandBuffer, bufferObj_.GetVkBuffer(), dstBuffer, 1, &bufferCopy);

    return VK_SUCCESS;
}

VkResult VKStagingBuffer::WriteAndIncrementOffset(
    VkDevice        device,
    VkCommandBuffer commandBuffer,
    VkBuffer        dstBuffer,
    VkDeviceSize    dstOffset,
    const void*     data,
    VkDeviceSize    dataSize)
{
    VkResult result = Write(device, commandBuffer, dstBuffer, dstOffset, data, dataSize);
    offset_ += dataSize;
    return result;
}


} // /namespace LLGL



// ================================================================================
