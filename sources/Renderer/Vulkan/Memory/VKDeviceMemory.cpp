/*
 * VKDeviceMemory.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKDeviceMemory.h"
#include "../VKCore.h"
#include "../../../Core/Helper.h"


namespace LLGL
{


VKDeviceMemory::VKDeviceMemory(const VKPtr<VkDevice>& device, VkDeviceSize size, std::uint32_t memoryTypeIndex) :
    deviceMemory_    { device, vkFreeMemory },
    size_            { size                 },
    memoryTypeIndex_ { memoryTypeIndex      },
    maxNewBlockSize_ { size                 }
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

    if (result != VK_SUCCESS)
    {
        std::string info = "failed to allocate Vulkan device memory of " + std::to_string(size) + " bytes";
        VKThrowIfFailed(result, info.c_str());
    }
}

void* VKDeviceMemory::Map(VkDevice device, VkDeviceSize offset, VkDeviceSize size)
{
    void* data = nullptr;

    auto result = vkMapMemory(device, deviceMemory_, offset, size, 0, &data);
    VKThrowIfFailed(result, "failed to map Vulkan buffer into CPU memory space");

    return data;
}

void VKDeviceMemory::Unmap(VkDevice device)
{
    vkUnmapMemory(device, deviceMemory_);
}

VKDeviceMemoryRegion* VKDeviceMemory::Allocate(VkDeviceSize size, VkDeviceSize alignment)
{
    if (size > 0 && alignment > 0)
    {
        /* Adjust size and offset by alignment */
        auto alignedSize    = GetAlignedSize(size, alignment);
        auto alignedOffset  = GetAlignedSize(GetNextOffset(), alignment);

        /* Allocate block with aligned size and offset */
        if (alignedSize + alignedOffset <= GetSize())
            return AllocBlock(alignedSize, alignedOffset);

        /* Reuse fragmanted block */
        if (alignedSize <= maxFragmentedBlockSize_)
            return FindReusableBlock(alignedSize, alignment);
    }
    return nullptr;
}

void VKDeviceMemory::Release(VKDeviceMemoryRegion* region)
{
    if (region)
    {
        /* Increase maximal size of fragmented blocks */
        maxFragmentedBlockSize_ = std::max(maxFragmentedBlockSize_, region->GetSize());
        InsertBlockToFragmentsSorted(region);
    }
}

bool VKDeviceMemory::IsEmpty() const
{
    return blocks_.empty();
}

VkDeviceSize VKDeviceMemory::GetMaxAllocationSize() const
{
    return std::max(maxNewBlockSize_, maxFragmentedBlockSize_);
}

void VKDeviceMemory::AccumDetails(VKDeviceMemoryDetails& details) const
{
    details.numChunks               += 1;
    details.numBlocks               += blocks_.size();
    details.numFragments            += fragmentedBlocks_.size();
    details.maxNewBlockSize         = std::max(details.maxNewBlockSize, maxNewBlockSize_);
    details.maxFragmentedBlockSize  = std::max(details.maxFragmentedBlockSize, maxFragmentedBlockSize_);
}


/*
 * ======= Private: =======
 */

VkDeviceSize VKDeviceMemory::GetNextOffset() const
{
    if (!blocks_.empty())
    {
        auto block = blocks_.back().get();
        return (block->GetOffset() + block->GetSize());
    }
    return 0;
}

VKDeviceMemoryRegion* VKDeviceMemory::AllocBlock(VkDeviceSize alignedSize, VkDeviceSize alignedOffset)
{
    /* Update maximal free block size */
    maxNewBlockSize_ = GetSize() - (alignedSize + alignedOffset);

    /* Allocate new block */
    return TakeOwnership(blocks_, MakeUnique<VKDeviceMemoryRegion>(this, alignedSize, alignedOffset, memoryTypeIndex_));
}

VKDeviceMemoryRegion* VKDeviceMemory::FindReusableBlock(VkDeviceSize alignedSize, VkDeviceSize alignment)
{
    /* Search for fragmented block that fits the requirement */
    for (auto it = fragmentedBlocks_.begin(); it != fragmentedBlocks_.end(); ++it)
    {
        auto block = *it;
        
        /* Check if required size plus the additional aligned offset fits into the current block */
        const auto alignedOffset = GetAlignedSize(block->GetOffset(), alignment);
        if (alignedSize + alignedOffset - block->GetOffset() <= block->GetSize())
        {
            /* If this block has the maximum fragmanted block size, this value must be updated */
            const bool mustUpdateMaxSize = (block->GetSize() == maxFragmentedBlockSize_);

            /* If block can be reused with its entire size, it can be removed from the fragmented blocks */

            //TODO...

            if (mustUpdateMaxSize)
                UpdateMaxFragmantedBlockSize();

            /* Reuse current block */
            return block;
        }
    }

    /* No reusable block found */
    return nullptr;
}

void VKDeviceMemory::UpdateMaxFragmantedBlockSize()
{
    maxFragmentedBlockSize_ = 0;
    for (auto block : fragmentedBlocks_)
        maxFragmentedBlockSize_ = std::max(maxFragmentedBlockSize_, block->GetSize());
}

void VKDeviceMemory::InsertBlockToFragmentsSorted(VKDeviceMemoryRegion* region)
{
    auto offset = region->GetOffset();
    auto count  = fragmentedBlocks_.size();

    /* Start serach in half-open range [begin, end) */
    decltype(count) begin = 0, end = count;

    while (begin + 2 < end)
    {
        /* Check if region's offset is before or after the middle's element */
        auto center = (begin + end) / 2;

        if (offset < fragmentedBlocks_[center]->GetOffset())
        {
            /* Restrict search to lower half */
            end = center + 1;
        }
        else
        {
            /* Restrict search to upper half */
            begin = center;
        }
    }

    /* Insert region by insertion-sort of the remaining half-open range [begin, begin + 2) */
    if (begin < count && offset < fragmentedBlocks_[begin]->GetOffset())
        InsertBlockToFragmentsAt(region, begin);
    else if (begin + 1 < count)
    {
        if (offset < fragmentedBlocks_[begin + 1]->GetOffset())
            InsertBlockToFragmentsAt(region, begin + 1);
        else
            InsertBlockToFragmentsAt(region, begin + 2);
    }
    else
        InsertBlockToFragmentsAt(region, count);
}

void VKDeviceMemory::InsertBlockToFragmentsAt(VKDeviceMemoryRegion* region, std::size_t position)
{
    auto count = fragmentedBlocks_.size();

    /* Try to merge region into lower part: [LOWER][BLOCK] --> [+++LOWER++++] */
    if (position > 0 && fragmentedBlocks_[position - 1]->MergeWith(*region))
    {
        /* Try to merge upper part into lower part: [+++LOWER++++][UPPER] --> [+++++++LOWER+++++++] */
        if (position < count && fragmentedBlocks_[position - 1]->MergeWith(*fragmentedBlocks_[position]))
        {
            /* Remove previous upper part after two-phase-merge */
            fragmentedBlocks_.erase(fragmentedBlocks_.begin() + position);
        }
    }
    /* Try to merge region into upper part: [BLOCK][UPPER] --> [+++UPPER++++] */
    else if (position > count || !fragmentedBlocks_[position]->MergeWith(*region))
    {
        /* Insert new fragmented block: [LOWER]..[BLOCK]..[UPPER] */
        fragmentedBlocks_.insert(fragmentedBlocks_.begin() + position, region);
    }
}

VkDeviceSize VKDeviceMemory::GetFragmentBlockOffsetEnd(std::size_t position) const
{
    auto block = fragmentedBlocks_[position];
    return (block->GetOffset() + block->GetSize());
}


} // /namespace LLGL



// ================================================================================
