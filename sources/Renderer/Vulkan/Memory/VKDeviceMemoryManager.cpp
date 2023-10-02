/*
 * VKDeviceMemoryManager.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "VKDeviceMemoryManager.h"
#include "../VKCore.h"
#include "../../ContainerTypes.h"


namespace LLGL
{


VKDeviceMemoryManager::VKDeviceMemoryManager(
    VkDevice                                device,
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
    const VkDeviceSize  alignedSize     = GetAlignedSize(size, alignment);
    const VkDeviceSize  allocationSize  = std::max(minAllocationSize_, alignedSize);
    const std::uint32_t memoryTypeIndex = FindMemoryType(memoryTypeBits, properties);

    if (VKDeviceMemory* chunk = FindOrAllocChunk(allocationSize, memoryTypeIndex, alignedSize))
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
        if (VKDeviceMemory* chunk = region->GetParentChunk())
        {
            /* Release block in chunk */
            chunk->Release(region);

            /* Release chunk if it's empty */
            if (chunk->IsEmpty())
                chunks_.erase(chunk);
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
    return chunks_.emplace<VKDeviceMemory>(device_, size, memoryTypeIndex);
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
