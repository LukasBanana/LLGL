/*
 * VKStagingBuffer.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_VK_STAGING_BUFFER_H
#define LLGL_VK_STAGING_BUFFER_H


#include "VKDeviceBuffer.h"


namespace LLGL
{


class VKDeviceMemoryManager;

class VKStagingBuffer
{

    public:

        // Creates the native D3D upload resource.
        VKStagingBuffer(
            VKDeviceMemoryManager&  deviceMemoryMngr,
            VkDeviceSize            size,
            VkDeviceSize            alignment           = 256u,
            VkMemoryPropertyFlags   memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        );

        VKStagingBuffer(VKStagingBuffer&& rhs) noexcept;
        VKStagingBuffer& operator = (VKStagingBuffer&& rhs) noexcept;

        VKStagingBuffer(const VKStagingBuffer&) = delete;
        VKStagingBuffer& operator = (const VKStagingBuffer&) = delete;

        // Creates a new resource and resets the writing offset.
        void Create(
            VKDeviceMemoryManager&  deviceMemoryMngr,
            VkDeviceSize            size,
            VkDeviceSize            alignment           = 256u,
            VkMemoryPropertyFlags   memoryPropertyFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
        );

        // Resets the writing offset.
        void Reset();

        // Returns true if the remaining buffer size can fit the specified data size.
        bool Capacity(VkDeviceSize dataSize) const;

        // Writes the specified data to the native Vulkan upload buffer.
        VkResult Write(
            VkDevice        device,
            VkCommandBuffer commandBuffer,
            VkBuffer        dstBuffer,
            VkDeviceSize    dstOffset,
            const void*     data,
            VkDeviceSize    dataSize
        );

        // Writes the specified data to the native Vulkan upload buffer and increments the write offset.
        VkResult WriteAndIncrementOffset(
            VkDevice        device,
            VkCommandBuffer commandBuffer,
            VkBuffer        dstBuffer,
            VkDeviceSize    dstOffset,
            const void*     data,
            VkDeviceSize    dataSize
        );

        // Returns the hardware buffer object.
        inline VkBuffer GetVkBuffer() const
        {
            return bufferObj_.GetVkBuffer();
        }

        // Returns the size of the native D3D buffer.
        inline VkDeviceSize GetSize() const
        {
            return size_;
        }

        // Returns the current writing offset.
        inline VkDeviceSize GetOffset() const
        {
            return offset_;
        }

    private:

        VKDeviceBuffer  bufferObj_;
        VkDeviceSize    size_       = 0;
        VkDeviceSize    offset_     = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
