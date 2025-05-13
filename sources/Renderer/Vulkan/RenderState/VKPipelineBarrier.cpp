/*
 * VKPipelineBarrier.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "VKPipelineBarrier.h"
#include "../Buffer/VKBuffer.h"
#include "../Texture/VKTexture.h"
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
#if 0 //UNUSED
        static_cast<std::uint32_t>(memoryBarriers_.size()),
        memoryBarriers_.data(),
#else
        0,
        nullptr,
#endif
        static_cast<std::uint32_t>(bufferBarriers_.size()),
        bufferBarriers_.data(),
        static_cast<std::uint32_t>(imageBarriers_.size()),
        imageBarriers_.data()
    );
}

std::uint32_t VKPipelineBarrier::AllocateBufferBarrier(VkPipelineStageFlags stageFlags)
{
    const std::uint32_t index = static_cast<std::uint32_t>(bufferBarriers_.size());
    InsertBufferMemoryBarrier(stageFlags, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_NULL_HANDLE);
    return index;
}

std::uint32_t VKPipelineBarrier::AllocateImageBarrier(VkPipelineStageFlags stageFlags)
{
    const std::uint32_t index = static_cast<std::uint32_t>(imageBarriers_.size());
    InsertImageMemoryBarrier(stageFlags, VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_NULL_HANDLE);
    return index;
}

void VKPipelineBarrier::SetBufferBarrier(std::uint32_t index, VkBuffer buffer)
{
    if (index < bufferBarriers_.size())
        bufferBarriers_[index].buffer = buffer;
}

void VKPipelineBarrier::SetImageBarrier(std::uint32_t index, VkImage image)
{
    if (index < imageBarriers_.size())
    {
        imageBarriers_[index].image = image;
    }
}


/*
 * ======= Private: =======
 */

#if 0 //UNUSED
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
#endif

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

//TODO: this is incomplete!
void VKPipelineBarrier::InsertImageMemoryBarrier(VkPipelineStageFlags stageFlags, VkAccessFlags srcAccess, VkAccessFlags dstAccess, VkImage image)
{
    VkImageMemoryBarrier barrier;
    {
        barrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.pNext                           = nullptr;
        barrier.srcAccessMask                   = srcAccess;
        barrier.dstAccessMask                   = dstAccess;
        barrier.oldLayout                       = VK_IMAGE_LAYOUT_UNDEFINED; // ???
        barrier.newLayout                       = VK_IMAGE_LAYOUT_UNDEFINED; // ???
        barrier.srcQueueFamilyIndex             = 0;
        barrier.dstQueueFamilyIndex             = 0;
        barrier.image                           = image;
        barrier.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.baseMipLevel   = 0;
        barrier.subresourceRange.levelCount     = 0;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount     = 0;
    }
    imageBarriers_.push_back(barrier);
}


} // /namespace LLGL



// ================================================================================
