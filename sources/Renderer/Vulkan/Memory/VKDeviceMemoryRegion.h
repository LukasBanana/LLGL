/*
 * VKDeviceMemoryRegion.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_VK_DEVICE_MEMORY_REGION_H
#define LLGL_VK_DEVICE_MEMORY_REGION_H


#include <vulkan/vulkan.h>
#include <cstdint>


namespace LLGL
{


class VKDeviceMemory;

// An instance of this class represents an atomic region within a VkDeviceMemory allocation.
class VKDeviceMemoryRegion
{

    public:

        VKDeviceMemoryRegion(VKDeviceMemory* deviceMemory, VkDeviceSize alignedSize, VkDeviceSize alignedOffset, std::uint32_t memoryTypeIndex);

        // Binds the specified buffer to this memory region.
        void BindBuffer(VkDevice device, VkBuffer buffer);

        // Binds the specified image to this memory region.
        void BindImage(VkDevice device, VkImage image);

        // Returns the parent device memory chunk.
        inline VKDeviceMemory* GetParentChunk() const
        {
            return deviceMemory_;
        }

        // Returns the aligned size.
        inline VkDeviceSize GetSize() const
        {
            return size_;
        }

        // Returns the aligned offset.
        inline VkDeviceSize GetOffset() const
        {
            return offset_;
        }

        // Returns the aligned offset with the added size.
        inline VkDeviceSize GetOffsetWithSize() const
        {
            return offset_ + size_;
        }

        // Returns the memory type index.
        inline std::uint32_t GetMemoryTypeIndex() const
        {
            return memoryTypeIndex_;
        }

    protected:

        friend class VKDeviceMemory;

        // Tries to merge the specified region into this region, and returns true on success.
        bool MergeWith(VKDeviceMemoryRegion& other);

        // Sets the new size and offset.
        void MoveAt(VkDeviceSize alignedSize, VkDeviceSize alignedOffset);

    private:

        VKDeviceMemory* deviceMemory_       = nullptr;
        VkDeviceSize    size_               = 0;
        VkDeviceSize    offset_             = 0;
        std::uint32_t   memoryTypeIndex_    = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
