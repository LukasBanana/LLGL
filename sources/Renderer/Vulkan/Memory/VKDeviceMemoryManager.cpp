/*
 * VKDeviceMemoryManager.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKDeviceMemoryManager.h"
#include "../VKCore.h"
#include "../../../Core/Helper.h"


namespace LLGL
{


VKDeviceMemoryManager::VKDeviceMemoryManager(
    const VKPtr<VkDevice>&                  device,
    const VkPhysicalDeviceMemoryProperties& memoryProperties,
    VkDeviceSize                            minAllocationSize,
    bool                                    reduceFragmentation)
:
    device_              { device              },
    memoryProperties_    { memoryProperties    },
    minAllocationSize_   { minAllocationSize   },
    reduceFragmentation_ { reduceFragmentation }
{
}

VKDeviceMemoryRegion* VKDeviceMemoryManager::Allocate(
    VkDeviceSize            size,
    VkDeviceSize            alignment,
    std::uint32_t           memoryTypeBits,
    VkMemoryPropertyFlags   properties)
{
    const auto alignedSize      = GetAlignedSize(size, alignment);
    const auto memoryTypeIndex  = FindMemoryType(memoryTypeBits, properties);
    const auto allocationSize   = std::max(minAllocationSize_, alignedSize);

    if (auto chunk = FindOrAllocChunk(allocationSize, memoryTypeIndex, alignedSize))
        return chunk->Allocate(size, alignment);
    else
        return nullptr;
}

VKDeviceMemoryRegion* VKDeviceMemoryManager::Allocate(
    const VkMemoryRequirements& requirements,
    VkMemoryPropertyFlags       properties)
{
    return Allocate(
        requirements.size,
        requirements.alignment,
        requirements.memoryTypeBits,
        properties
    );
}

void VKDeviceMemoryManager::Release(VKDeviceMemoryRegion* region)
{
    if (region)
    {
        if (auto chunk = region->GetParentChunk())
        {
            /* Release block in chunk */
            chunk->Release(region);

            /* Release chunk if it's empty */
            if (chunk->IsEmpty())
            {
                RemoveFromListIf(
                    chunks_,
                    [chunk](std::unique_ptr<VKDeviceMemory>& entry)
                    {
                        return (entry.get() == chunk);
                    }
                );
            }
        }
    }
}

VKDeviceMemoryDetails VKDeviceMemoryManager::QueryDetails() const
{
    VKDeviceMemoryDetails details;
    {
        for (const auto& chunk : chunks_)
            chunk->AccumDetails(details);
    }
    return details;
}

#ifdef LLGL_DEBUG

void VKDeviceMemoryManager::PrintBlocks(std::ostream& s, const std::string& title) const
{
    std::size_t i = 0;
    for (const auto& chunk : chunks_)
    {
        s << "chunk[" << (i++) << "]:";

        if (!title.empty())
            s << " \"" << title << '\"';

        s << '\n';
        s << "  size             = " << chunk->GetSize() << '\n';
        s << "  memoryTypeIndex  = " << chunk->GetMemoryTypeIndex() << '\n';

        s << "  blocks           = ";
        chunk->PrintBlocks(s);
        s << '\n';

        s << "  fragmentedBlocks = ";
        chunk->PrintFragmentedBlocks(s);
        s << '\n';
    }
}

#endif


/*
 * ======= Private: =======
 */

std::uint32_t VKDeviceMemoryManager::FindMemoryType(std::uint32_t memoryTypeBits, VkMemoryPropertyFlags properties) const
{
    return VKFindMemoryType(memoryProperties_, memoryTypeBits, properties);
}

VKDeviceMemory* VKDeviceMemoryManager::AllocChunk(VkDeviceSize size, std::uint32_t memoryTypeIndex)
{
    return TakeOwnership(chunks_, MakeUnique<VKDeviceMemory>(device_, size, memoryTypeIndex));
}

VKDeviceMemory* VKDeviceMemoryManager::FindOrAllocChunk(VkDeviceSize allocationSize, std::uint32_t memoryTypeIndex, VkDeviceSize minFreeBlockSize)
{
    /* Search for a suitable chunk */
    for (const auto& chunk : chunks_)
    {
        if (chunk->GetMaxAllocationSize() >= minFreeBlockSize && chunk->GetMemoryTypeIndex() == memoryTypeIndex)
            return chunk.get();
    }

    /* Allocate new chunk */
    return AllocChunk(allocationSize, memoryTypeIndex);
}


} // /namespace LLGL



// ================================================================================
