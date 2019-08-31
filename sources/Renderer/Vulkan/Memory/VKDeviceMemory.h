/*
 * VKDeviceMemory.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_VK_DEVICE_MEMORY_H
#define LLGL_VK_DEVICE_MEMORY_H


#include "VKDeviceMemoryRegion.h"
#include "../VKPtr.h"
#include <vulkan/vulkan.h>
#include <cstdint>
#include <vector>
#include <memory>

#ifdef LLGL_DEBUG
#   include <ostream>
#endif


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

        VKDeviceMemory(const VKDeviceMemory&) = delete;
        VKDeviceMemory& operator = (const VKDeviceMemory&) = delete;

        VKDeviceMemory(VKDeviceMemory&&) = default;
        VKDeviceMemory& operator = (VKDeviceMemory&&) = default;

        void* Map(VkDevice device, VkDeviceSize offset, VkDeviceSize size);
        void Unmap(VkDevice device);

        // Tries to allocate a new block within this device memory chunk, and returns null of failure.
        VKDeviceMemoryRegion* Allocate(VkDeviceSize size, VkDeviceSize alignment, bool reduceFragmentation = false);

        // Releases the specified block within this device memory chunk.
        void Release(VKDeviceMemoryRegion* region);

        // Returns true if this device memory has no more blocks.
        bool IsEmpty() const;

        // Returns the maximal size that can be allocated for a device memory region within this device memory chunk.
        VkDeviceSize GetMaxAllocationSize() const;

        // Accumulates the memory details of this device memory into the output structure.
        void AccumDetails(VKDeviceMemoryDetails& details) const;

        #ifdef LLGL_DEBUG

        void PrintBlocks(std::ostream& s) const;
        void PrintFragmentedBlocks(std::ostream& s) const;

        #endif

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

        // Makes a new device memory block.
        std::unique_ptr<VKDeviceMemoryRegion> MakeUniqueBlock(VkDeviceSize alignedSize, VkDeviceSize alignedOffset);

        // Allocates a new block.
        VKDeviceMemoryRegion* AllocAndAppendBlock(VkDeviceSize alignedSize, VkDeviceSize alignedOffset);

        // Inserts the specified region into the main block list by insertion-sort.
        VKDeviceMemoryRegion* InsertBlock(std::unique_ptr<VKDeviceMemoryRegion>&& region);

        // Tries to find a fragmented block that can be reused.
        VKDeviceMemoryRegion* FindReusableBlock(VkDeviceSize alignedSize, VkDeviceSize alignment);

        // Updates the maximal size of fragmanted blocks.
        void UpdateMaxFragmentedBlockSize();

        // Inserts the specified region into the fragmented block list by insertion-sort.
        void InsertBlockToFragmentsSorted(std::unique_ptr<VKDeviceMemoryRegion>&& region);

        // Inserts the specified region into the fragmented block list at the specified position and merges it with surrounding blocks if possible.
        void InsertBlockToFragmentsAt(std::unique_ptr<VKDeviceMemoryRegion>&& region, std::size_t position);

        // Merges the two fragmented blocks and records the maximal fragmented block size.
        bool MergeFragmentedBlockWith(VKDeviceMemoryRegion& region, VKDeviceMemoryRegion& appendixRegion);

        // Returns the offset after the specified fragmented block.
        VkDeviceSize GetFragmentBlockOffsetEnd(std::size_t position) const;

        // Increases the maximal fragmented block size.
        void IncMaxFragmentedBlockSize(VkDeviceSize size);

        VKPtr<VkDeviceMemory>                               deviceMemory_;
        VkDeviceSize                                        size_                   = 0;
        std::uint32_t                                       memoryTypeIndex_        = 0;

        VkDeviceSize                                        maxNewBlockSize_        = 0;
        std::vector<std::unique_ptr<VKDeviceMemoryRegion>>  blocks_;

        VkDeviceSize                                        maxFragmentedBlockSize_ = 0;
        std::vector<std::unique_ptr<VKDeviceMemoryRegion>>  fragmentedBlocks_;

};


} // /namespace LLGL


#endif



// ================================================================================
