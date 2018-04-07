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
#include <vector>
#include <memory>


namespace LLGL
{


// Details structure of VKDeviceMemory for debugging.
struct VKDeviceMemoryDetails
{
    std::size_t     numChunks               = 0;
    std::size_t     numBlocks               = 0;
    std::size_t     numFragments            = 0;
    VkDeviceSize    maxNewBlockSize         = 0;
    VkDeviceSize    maxFragmentedBlockSize  = 0;
};

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

        // Returns the maximal size that can be allocated for a device memory region within this device memory chunk.
        VkDeviceSize GetMaxAllocationSize() const;

        // Accumulates the memory details of this device memory into the output structure.
        void AccumDetails(VKDeviceMemoryDetails& details) const;

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

    private:

        // Returns the next offset after the last block.
        VkDeviceSize GetNextOffset() const;

        // Allocates a new block.
        VKDeviceMemoryRegion* AllocBlock(VkDeviceSize alignedSize, VkDeviceSize alignedOffset);

        // Tries to find a fragmented block that can be reused.
        VKDeviceMemoryRegion* FindReusableBlock(VkDeviceSize alignedSize, VkDeviceSize alignment);

        // Updates the maximal size of fragmanted blocks.
        void UpdateMaxFragmantedBlockSize();

        // Inserts the specified region into the fragmented block list by insertion-sort.
        void InsertBlockToFragmentsSorted(VKDeviceMemoryRegion* region);

        // Inserts the specified region into the fragmented block list at the specified position and merges it with surrounding blocks if possible.
        void InsertBlockToFragmentsAt(VKDeviceMemoryRegion* region, std::size_t position);

        // Returns the offset after the specified fragmented block.
        VkDeviceSize GetFragmentBlockOffsetEnd(std::size_t position) const;

        VKPtr<VkDeviceMemory>                               deviceMemory_;
        VkDeviceSize                                        size_                   = 0;
        std::uint32_t                                       memoryTypeIndex_        = 0;

        VkDeviceSize                                        maxNewBlockSize_        = 0;
        std::list<std::unique_ptr<VKDeviceMemoryRegion>>    blocks_;

        VkDeviceSize                                        maxFragmentedBlockSize_ = 0;
        std::vector<VKDeviceMemoryRegion*>                  fragmentedBlocks_;

};


} // /namespace LLGL


#endif



// ================================================================================
