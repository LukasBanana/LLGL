/*
 * VKDeviceMemory.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "VKDeviceMemory.h"
#include "../VKCore.h"
#include "../../ContainerTypes.h"

#include "../../../Core/Assertion.h"


namespace LLGL
{


VKDeviceMemory::VKDeviceMemory(VkDevice device, VkDeviceSize size, std::uint32_t memoryTypeIndex) :
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
    VkResult result = vkAllocateMemory(device, &allocInfo, nullptr, deviceMemory_.ReleaseAndGetAddressOf());

    if (result != VK_SUCCESS)
    {
        std::string info = "failed to allocate Vulkan device memory of " + std::to_string(size) + " bytes";
        VKThrowIfFailed(result, info.c_str());
    }
}

void* VKDeviceMemory::Map(VkDevice device, VkDeviceSize offset, VkDeviceSize size)
{
    void* data = nullptr;

    VkResult result = vkMapMemory(device, deviceMemory_, offset, size, 0, &data);
    VKThrowIfFailed(result, "failed to map Vulkan buffer into CPU memory space");

    return data;
}

void VKDeviceMemory::Unmap(VkDevice device)
{
    vkUnmapMemory(device, deviceMemory_);
}

VKDeviceMemoryRegion* VKDeviceMemory::Allocate(VkDeviceSize size, VkDeviceSize alignment, bool reduceFragmentation)
{
    if (size > 0 && alignment > 0)
    {
        /* Adjust size and offset by alignment */
        VkDeviceSize alignedSize    = GetAlignedSize(size, alignment);
        VkDeviceSize alignedOffset  = GetAlignedSize(GetNextOffset(), alignment);

        if (reduceFragmentation)
        {
            /* Reuse fragmented block */
            if (alignedSize <= maxFragmentedBlockSize_)
                return FindReusableBlock(alignedSize, alignment);

            /* Allocate block with aligned size and offset */
            if (alignedSize + alignedOffset <= GetSize())
                return AllocAndAppendBlock(alignedSize, alignedOffset);
        }
        else
        {
            /* Allocate block with aligned size and offset */
            if (alignedSize + alignedOffset <= GetSize())
                return AllocAndAppendBlock(alignedSize, alignedOffset);

            /* Reuse fragmented block */
            if (alignedSize <= maxFragmentedBlockSize_)
                return FindReusableBlock(alignedSize, alignment);
        }
    }
    return nullptr;
}

void VKDeviceMemory::Release(VKDeviceMemoryRegion* region)
{
    if (region)
    {
        /* Increase maximal size of fragmented blocks */
        IncMaxFragmentedBlockSize(region->GetSize());

        auto it = std::find_if(
            blocks_.begin(), blocks_.end(),
            [region](const std::unique_ptr<VKDeviceMemoryRegion>& entry)
            {
                return (entry.get() == region);
            }
        );

        if (it != blocks_.end())
        {
            InsertBlockToFragmentsSorted(std::move(*it));
            blocks_.erase(it);
        }
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

#ifdef LLGL_DEBUG

/*
Prints a single memory region to the output stream.
Example of 3 consecutive blocks: [0+++++][8++][13++++++]
Example of 3 fragmented blocks: [0+++++]...[11+].[17++++++]
*/
static void PrintDeviceMemoryRegion(std::ostream& s, VKDeviceMemoryRegion& region, VKDeviceMemoryRegion* prevRegion)
{
    /* Print space between previous and current region */
    if (prevRegion)
    {
        auto startOffset    = prevRegion->GetOffsetWithSize();
        auto endOffset      = region.GetOffset();
        if (startOffset < endOffset)
            s << std::string(static_cast<std::size_t>(endOffset - startOffset), '.');
    }
    else
    {
        auto endOffset = region.GetOffset();
        if (endOffset > 0)
            s << std::string(static_cast<std::size_t>(endOffset), '.');
    }

    /* Print new region */
    auto n = static_cast<std::size_t>(region.GetSize());
    if (n > 2)
    {
        s << '[';

        auto numStr = std::to_string(region.GetSize());

        n -= 2;
        if (numStr.size() <= n)
        {
            s << numStr;
            n -= numStr.size();
        }

        if (n > 0)
            s << std::string(n, '+');

        s << ']';
    }
    else if (n == 2)
        s << "[]";
    else if (n == 1)
        s << '|';
}

void VKDeviceMemory::PrintBlocks(std::ostream& s) const
{
    VKDeviceMemoryRegion* prevBlock = nullptr;
    for (const auto& block : blocks_)
    {
        PrintDeviceMemoryRegion(s, *block, prevBlock);
        prevBlock = block.get();
    }
}

void VKDeviceMemory::PrintFragmentedBlocks(std::ostream& s) const
{
    VKDeviceMemoryRegion* prevBlock = nullptr;
    for (const auto& block : fragmentedBlocks_)
    {
        PrintDeviceMemoryRegion(s, *block, prevBlock);
        prevBlock = block.get();
    }
}

#endif


/*
 * ======= Private: =======
 */

VkDeviceSize VKDeviceMemory::GetNextOffset() const
{
    if (blocks_.empty())
        return 0;
    else
        return blocks_.back()->GetOffsetWithSize();
}

std::unique_ptr<VKDeviceMemoryRegion> VKDeviceMemory::MakeUniqueBlock(VkDeviceSize alignedSize, VkDeviceSize alignedOffset)
{
    return MakeUnique<VKDeviceMemoryRegion>(this, alignedSize, alignedOffset, memoryTypeIndex_);
}

VKDeviceMemoryRegion* VKDeviceMemory::AllocAndAppendBlock(VkDeviceSize alignedSize, VkDeviceSize alignedOffset)
{
    /* Update maximal free block size */
    maxNewBlockSize_ = GetSize() - (alignedSize + alignedOffset);

    /* Check if the aligned offset results in a new fragment */
    const VkDeviceSize nextOffset = GetNextOffset();

    if (nextOffset < alignedOffset)
    {
        /*
        Check if new fragment can be immediately merged with previous fragment:
        Before: blocks:    [++]........[++++]
                fragments: ....[++++]
        After:  blocks:    [++]........[++++]
                fragments: ....[++++++]
        */
        if (!fragmentedBlocks_.empty() && fragmentedBlocks_.back()->GetOffset() == nextOffset)
        {
            /* Resize last fragmented block (can also get smaller) */
            const VkDeviceSize size = fragmentedBlocks_.back()->GetSize();
            fragmentedBlocks_.back()->MoveAt(alignedOffset - nextOffset, nextOffset);
            DecMaxFragmentedBlockSize(size);
        }
        else
        {
            /* Append new fragmented block at the end */
            fragmentedBlocks_.push_back(MakeUniqueBlock(alignedOffset - nextOffset, nextOffset));
        }

        /* Track largest fragmented block size */
        IncMaxFragmentedBlockSize(fragmentedBlocks_.back()->GetSize());
    }
    else
    {
        /* Check if last fragmented block can be replaced by new block */
        if (!fragmentedBlocks_.empty() && fragmentedBlocks_.back()->GetOffset() == nextOffset)
        {
            /* Move fragmented block into main blocks and resize it */
            VKDeviceMemoryRegion* block = TakeOwnership(blocks_, PopBackFragmentedBlock());
            block->MoveAt(alignedSize, alignedOffset);
            return block;
        }
    }

    /* Allocate new block */
    return TakeOwnership(blocks_, MakeUniqueBlock(alignedSize, alignedOffset));
}

VKDeviceMemoryRegion* VKDeviceMemory::InsertBlock(std::unique_ptr<VKDeviceMemoryRegion>&& region)
{
    VKDeviceMemoryRegion* regionRef = region.get();

    /* Add block by insertion sort */
    if (!blocks_.empty())
    {
        for (auto it = blocks_.rbegin(); it != blocks_.rend(); ++it)
        {
            if ((*it)->GetOffset() < region->GetOffset())
            {
                blocks_.insert(it.base(), std::move(region));
                break;
            }
        }
    }
    else
        blocks_.emplace_back(std::move(region));

    return regionRef;
}

VKDeviceMemoryRegion* VKDeviceMemory::FindReusableBlock(VkDeviceSize alignedSize, VkDeviceSize alignment)
{
    VKDeviceMemoryRegion* prevBlock = nullptr;
    VKDeviceMemoryRegion* nextBlock = nullptr;

    /* Search for fragmented block that fits the requirement */
    for (auto it = fragmentedBlocks_.begin(); it != fragmentedBlocks_.end(); ++it)
    {
        VKDeviceMemoryRegion* block = it->get();

        /* Check if required size plus the additional aligned offset fits into the current block */
        const VkDeviceSize alignedOffset = GetAlignedSize(block->GetOffset(), alignment);
        if (alignedSize + alignedOffset - block->GetOffset() <= block->GetSize())
        {
            /* If this block has the maximum fragmented block size, this value must be updated */
            const bool isLargestFragmentedBlock = (block->GetSize() == maxFragmentedBlockSize_);

            /* If block can be reused with its entire size, it can be removed from the fragmented blocks */
            InsertBlock(std::move(*it));

            it = fragmentedBlocks_.erase(it);
            if (it != fragmentedBlocks_.end())
                nextBlock = it->get();

            /* Check if block must be split up at the lower part */
            if (block->GetOffset() < alignedOffset)
            {
                /* Allocate fragmented lower block (left to the current block) */
                std::unique_ptr<VKDeviceMemoryRegion> blockLower = MakeUniqueBlock(alignedOffset - block->GetOffset(), block->GetOffset());

                if (!prevBlock || !MergeFragmentedBlockWith(*prevBlock, *blockLower))
                {
                    it = fragmentedBlocks_.insert(it, std::move(blockLower));
                    ++it;
                }
            }

            /* Check if block must be split up at the upper part */
            const VkDeviceSize alignedSizeWithOffset = alignedSize + alignedOffset - block->GetOffset();
            if (alignedSizeWithOffset < block->GetSize())
            {
                /* Allocate fragmented upper block (right to the current block) */
                const VkDeviceSize blockUpperSize = block->GetSize() - alignedSizeWithOffset;
                std::unique_ptr<VKDeviceMemoryRegion> blockUpper = MakeUniqueBlock(blockUpperSize, block->GetOffsetWithSize() - blockUpperSize);

                if (!nextBlock || !MergeFragmentedBlockWith(*blockUpper, *nextBlock))
                    fragmentedBlocks_.insert(it, std::move(blockUpper));
            }

            /* Move reused block to new offset and size */
            block->MoveAt(alignedSize, alignedOffset);

            if (isLargestFragmentedBlock)
                UpdateMaxFragmentedBlockSize();

            /* Reuse current block */
            return block;
        }

        prevBlock = block;
    }

    /* No reusable block found */
    return nullptr;
}

void VKDeviceMemory::UpdateMaxFragmentedBlockSize()
{
    maxFragmentedBlockSize_ = 0;
    for (const auto& block : fragmentedBlocks_)
        IncMaxFragmentedBlockSize(block->GetSize());
}

void VKDeviceMemory::InsertBlockToFragmentsSorted(std::unique_ptr<VKDeviceMemoryRegion>&& region)
{
    VkDeviceSize offset = region->GetOffset();
    std::size_t count = fragmentedBlocks_.size();

    /* Start serach in half-open range [begin, end) */
    std::size_t begin = 0, end = count;

    while (begin + 2 < end)
    {
        /* Check if region's offset is before or after the middle's element */
        std::size_t center = (begin + end) / 2;

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
        InsertBlockToFragmentsAt(std::move(region), begin);
    else if (begin + 1 < count)
    {
        if (offset < fragmentedBlocks_[begin + 1]->GetOffset())
            InsertBlockToFragmentsAt(std::move(region), begin + 1);
        else
            InsertBlockToFragmentsAt(std::move(region), begin + 2);
    }
    else
        InsertBlockToFragmentsAt(std::move(region), count);
}

void VKDeviceMemory::InsertBlockToFragmentsAt(std::unique_ptr<VKDeviceMemoryRegion>&& region, std::size_t position)
{
    std::size_t count = fragmentedBlocks_.size();

    /* Try to merge region into lower part: [LOWER][BLOCK] --> [+++LOWER++++] */
    if (position > 0 && MergeFragmentedBlockWith(*fragmentedBlocks_[position - 1], *region))
    {
        /* Try to merge upper part into lower part: [+++LOWER++++][UPPER] --> [+++++++LOWER+++++++] */
        if (position < count && MergeFragmentedBlockWith(*fragmentedBlocks_[position - 1], *fragmentedBlocks_[position]))
        {
            /* Remove previous upper part after two-phase-merge */
            const VkDeviceSize size = fragmentedBlocks_[position]->GetSize();
            fragmentedBlocks_.erase(fragmentedBlocks_.begin() + position);
            DecMaxFragmentedBlockSize(size);
        }
    }
    /* Try to merge region into upper part: [BLOCK][UPPER] --> [+++UPPER++++] */
    else if (position >= count || !MergeFragmentedBlockWith(*fragmentedBlocks_[position], *region))
    {
        /* Insert new fragmented block: [LOWER]..[BLOCK]..[UPPER] */
        IncMaxFragmentedBlockSize(region->GetSize());
        fragmentedBlocks_.insert(fragmentedBlocks_.begin() + position, std::move(region));
    }
}

std::unique_ptr<VKDeviceMemoryRegion> VKDeviceMemory::PopBackFragmentedBlock()
{
    std::unique_ptr<VKDeviceMemoryRegion> block = std::move(fragmentedBlocks_.back());
    fragmentedBlocks_.pop_back();
    DecMaxFragmentedBlockSize(block->GetSize());
    return block;
}

bool VKDeviceMemory::MergeFragmentedBlockWith(VKDeviceMemoryRegion& region, VKDeviceMemoryRegion& appendixRegion)
{
    if (region.MergeWith(appendixRegion))
    {
        IncMaxFragmentedBlockSize(region.GetSize());
        return true;
    }
    return false;
}

VkDeviceSize VKDeviceMemory::GetFragmentBlockOffsetEnd(std::size_t position) const
{
    return fragmentedBlocks_[position]->GetOffsetWithSize();
}

void VKDeviceMemory::IncMaxFragmentedBlockSize(VkDeviceSize size)
{
    maxFragmentedBlockSize_ = std::max(maxFragmentedBlockSize_, size);
}

void VKDeviceMemory::DecMaxFragmentedBlockSize(VkDeviceSize size)
{
    if (size == maxFragmentedBlockSize_)
        UpdateMaxFragmentedBlockSize();
}


} // /namespace LLGL



// ================================================================================
