/*
 * VKPipelineBarrier.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "VKPipelineBarrier.h"
#include "../Buffer/VKBuffer.h"
//#include "../Texture/VKTexture.h"
#include "../../CheckedCast.h"
#include "../../../Core/CoreUtils.h"
#include <LLGL/ShaderFlags.h>


namespace LLGL
{


bool VKPipelineBarrier::IsActive() const
{
    return (srcStageMask_ != 0 && dstStageMask_ != 0);
}

void VKPipelineBarrier::Submit(VkCommandBuffer commandBuffer)
{
    vkCmdPipelineBarrier(
        commandBuffer,
        srcStageMask_,
        dstStageMask_,
        0, // VkDependencyFlags
        static_cast<std::uint32_t>(memoryBarriers_.size()),
        memoryBarriers_.data(),
        static_cast<std::uint32_t>(bufferBarriers_.size()),
        bufferBarriers_.data(),
        static_cast<std::uint32_t>(imageBarriers_.size()),
        imageBarriers_.data()
    );
}

bool VKPipelineBarrier::Emplace(std::uint32_t slot, Resource* resource, VkPipelineStageFlags stageFlags)
{
    /* Emplace resource binding into sorted array */
    std::size_t index = 0;
    auto* entry = FindInSortedArray<ResourceBinding>(
        bindings_.data(),
        bindings_.size(),
        [slot](const ResourceBinding& binding) -> int
        {
            return (static_cast<int>(binding.slot) - static_cast<int>(slot));
        },
        &index
    );
    if (entry != nullptr)
    {
        /* Replace previous entry */
        if (entry->resource != resource)
        {
            entry->resource     = resource;
            entry->stageFlags   = stageFlags;
            return true;
        }
    }
    else
    {
        /* Insert entry with new binding slot */
        ResourceBinding binding;
        {
            binding.slot        = slot;
            binding.resource    = resource;
            binding.stageFlags  = stageFlags;
        }
        bindings_.insert(bindings_.begin() + index, binding);
        return true;
    }
    return false;
}

bool VKPipelineBarrier::Remove(std::uint32_t slot)
{
    /* Only clear resource entry, don't reallocate array */
    auto* entry = FindInSortedArray<ResourceBinding>(
        bindings_.data(),
        bindings_.size(),
        [slot](const ResourceBinding& binding) -> int
        {
            return (static_cast<int>(binding.slot) - static_cast<int>(slot));
        }
    );
    if (entry != nullptr && entry->resource != nullptr)
    {
        entry->resource = nullptr;
        return true;
    }
    return false;
}

bool VKPipelineBarrier::Update()
{
    /* Reset bitmasks and barriers */
    srcStageMask_ = 0;
    dstStageMask_ = 0;
    memoryBarriers_.clear();

    /* Iterate over all bindings and re-generate all barriers */
    for (const ResourceBinding& binding : bindings_)
    {
        if (Resource* resource = binding.resource)
        {
            if (resource->GetResourceType() == ResourceType::Buffer)
            {
                auto bufferVK = LLGL_CAST(VKBuffer*, resource);
                InsertBufferMemoryBarrier(binding.stageFlags, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, bufferVK->GetVkBuffer());
            }
            else
                InsertMemoryBarrier(binding.stageFlags, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
        }
    }

    return IsActive();
}


/*
 * ======= Private: =======
 */

void VKPipelineBarrier::InsertMemoryBarrier(VkPipelineStageFlags stageFlags, VkAccessFlags srcAccess, VkAccessFlags dstAccess)
{
    srcStageMask_ |= stageFlags;
    dstStageMask_ |= stageFlags;

    /* Check if a memory barrier alread exists */
    for (const VkMemoryBarrier& barrier : memoryBarriers_)
    {
        if (barrier.srcAccessMask == srcAccess && barrier.dstAccessMask == dstAccess)
            return;
    }

    /* Insert a new memory barrier */
    VkMemoryBarrier barrier;
    {
        barrier.sType           = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
        barrier.pNext           = nullptr;
        barrier.srcAccessMask   = srcAccess;
        barrier.dstAccessMask   = dstAccess;
    }
    memoryBarriers_.push_back(barrier);
}

void VKPipelineBarrier::InsertBufferMemoryBarrier(VkPipelineStageFlags stageFlags, VkAccessFlags srcAccess, VkAccessFlags dstAccess, VkBuffer buffer)
{
    srcStageMask_ |= stageFlags;
    dstStageMask_ |= stageFlags;

    VkBufferMemoryBarrier barrier;
    {
        barrier.sType                   = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
        barrier.pNext                   = nullptr;
        barrier.srcAccessMask           = srcAccess;
        barrier.dstAccessMask           = dstAccess;
        barrier.srcQueueFamilyIndex     = 0;
        barrier.dstQueueFamilyIndex     = 0;
        barrier.buffer                  = buffer;
        barrier.offset                  = 0;
        barrier.size                    = VK_WHOLE_SIZE;
    }
    bufferBarriers_.push_back(barrier);
}


} // /namespace LLGL



// ================================================================================
