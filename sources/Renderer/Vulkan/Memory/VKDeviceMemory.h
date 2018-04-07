/*
 * VKDeviceMemory.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_VK_DEVICE_MEMORY_H
#define LLGL_VK_DEVICE_MEMORY_H


#include "VKDeviceMemoryRegion.h"
#include "../VKPtr.h"
#include <vulkan/vulkan.h>
#include <cstdint>
#include <list>
#include <memory>


namespace LLGL
{


// An instance of this class holds a single VkDeviceMemory allocation chunk.
class VKDeviceMemory
{

    public:

        VKDeviceMemory(const VKPtr<VkDevice>& device, VkDeviceSize size, std::uint32_t memoryTypeIndex);

        void* Map(VkDevice device, VkDeviceSize offset, VkDeviceSize size);
        void Unmap(VkDevice device);

        // Tries to allocate a new block within this device memory chunk, and returns null of failure.
        VKDeviceMemoryRegion* Allocate(VkDeviceSize size, VkDeviceSize alignment);

        // Releases the specified block within this device memory chunk.
        void Release(VKDeviceMemoryRegion* region);

        // Returns true if this device memory has no more blocks.
        bool IsEmpty() const;

        // Returns the hardware buffer object.
        inline VkDeviceMemory GetVkDeviceMemory() const
        {
            return deviceMemory_.Get();
        }

        // Returns the size of the entire device memory chunk.
        inline VkDeviceSize GetSize() const
        {
            return size_;
        }

        // Returns the memory type index that was passed this device memory chunk was constructed.
        inline std::uint32_t GetMemoryTypeIndex() const
        {
            return memoryTypeIndex_;
        }

        // Returns the maximal size that can be allocated for a device memory region within this device memory chunk.
        inline VkDeviceSize GetMaxFreeBlockSize() const
        {
            return maxFreeBlockSize_;
        }

    private:

        VKDeviceMemoryRegion* AllocBlock(VkDeviceSize alignedSize, VkDeviceSize alignedOffset);

        VkDeviceSize GetNextOffset() const;

        VKPtr<VkDeviceMemory>                               deviceMemory_;
        VkDeviceSize                                        size_               = 0;
        std::uint32_t                                       memoryTypeIndex_    = 0;

        VkDeviceSize                                        maxFreeBlockSize_   = 0;

        std::list<std::unique_ptr<VKDeviceMemoryRegion>>    blocks_;

};


} // /namespace LLGL


#endif



// ================================================================================
