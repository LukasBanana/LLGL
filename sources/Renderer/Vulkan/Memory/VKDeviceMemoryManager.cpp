/*
 * VKDeviceMemoryManager.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKDeviceMemoryManager.h"
#include "../VKCore.h"
#include "../../../Core/Helper.h"


namespace LLGL
{


VKDeviceMemoryManager::VKDeviceMemoryManager(const VKPtr<VkDevice>& device, const VkPhysicalDeviceMemoryProperties& memoryProperties) :
    device_           { device           },
    memoryProperties_ { memoryProperties }
{
}

VKDeviceMemoryRegion* VKDeviceMemoryManager::Allocate(VkDeviceSize size, VkDeviceSize alignment, std::uint32_t memoryTypeBits, VkMemoryPropertyFlags properties)
{
    const auto memoryTypeIndex  = FindMemoryType(memoryTypeBits, properties);
    const auto allocationSize   = std::max(minAllocationSize_, size);

    if (auto chunk = FindOrAllocChunk(allocationSize, memoryTypeIndex, size))
        return chunk->Allocate(size, alignment);
    else
        return nullptr;
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
