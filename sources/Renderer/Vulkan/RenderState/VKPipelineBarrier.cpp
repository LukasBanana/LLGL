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
    const std::uint32_t nextIndex = static_cast<std::uint32_t>(bufferBarriers_.size());
    bufferBarriers_.resize(bufferBarriers_.size() + 1);
    InitializeBufferMemoryBarrier(bufferBarriers_.back(), VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
    MarkStageFlags(stageFlags);
    return nextIndex;
}

std::uint32_t VKPipelineBarrier::AllocateImageBarrier(VkPipelineStageFlags stageFlags)
{
    const std::uint32_t nextIndex = static_cast<std::uint32_t>(imageBarriers_.size());
    imageBarriers_.resize(imageBarriers_.size() + 1);
    InitializeImageMemoryBarrier(imageBarriers_.back(), VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT);
    MarkStageFlags(stageFlags);
    return nextIndex;
}

void VKPipelineBarrier::SetBufferBarrier(std::uint32_t index, VkBuffer buffer)
{
    if (index < bufferBarriers_.size())
        bufferBarriers_[index].buffer = buffer;
}

void VKPipelineBarrier::SetImageBarrier(std::uint32_t index, VkImage image)
{
    if (index < imageBarriers_.size())
        imageBarriers_[index].image = image;
}


/*
 * ======= Private: =======
 */

void VKPipelineBarrier::MarkStageFlags(VkPipelineStageFlags stageFlags)
{
    srcStageMask_ |= stageFlags;
    dstStageMask_ |= stageFlags;
}

#if 0 //UNUSED
void VKPipelineBarrier::InsertMemoryBarrier(VkPipelineStageFlags stageFlags, VkAccessFlags srcAccess, VkAccessFlags dstAccess)
{
    srcStageMask_ |= stageFlags;
    dstStageMask_ |= stageFlags;

    /* Check if a memory barrier already exists */
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

void VKPipelineBarrier::InitializeBufferMemoryBarrier(VkBufferMemoryBarrier& outBarrier, VkAccessFlags srcAccess, VkAccessFlags dstAccess)
{
    outBarrier.sType                = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    outBarrier.pNext                = nullptr;
    outBarrier.srcAccessMask        = srcAccess;
    outBarrier.dstAccessMask        = dstAccess;
    outBarrier.srcQueueFamilyIndex  = VK_QUEUE_FAMILY_IGNORED;
    outBarrier.dstQueueFamilyIndex  = VK_QUEUE_FAMILY_IGNORED;
    outBarrier.buffer               = VK_NULL_HANDLE;
    outBarrier.offset               = 0;
    outBarrier.size                 = VK_WHOLE_SIZE;
}

//TODO: this is incomplete!
void VKPipelineBarrier::InitializeImageMemoryBarrier(VkImageMemoryBarrier& outBarrier, VkAccessFlags srcAccess, VkAccessFlags dstAccess)
{
    outBarrier.sType                            = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    outBarrier.pNext                            = nullptr;
    outBarrier.srcAccessMask                    = srcAccess;
    outBarrier.dstAccessMask                    = dstAccess;
    outBarrier.oldLayout                        = VK_IMAGE_LAYOUT_UNDEFINED; // ???
    outBarrier.newLayout                        = VK_IMAGE_LAYOUT_UNDEFINED; // ???
    outBarrier.srcQueueFamilyIndex              = VK_QUEUE_FAMILY_IGNORED;
    outBarrier.dstQueueFamilyIndex              = VK_QUEUE_FAMILY_IGNORED;
    outBarrier.image                            = VK_NULL_HANDLE;
    outBarrier.subresourceRange.aspectMask      = VK_IMAGE_ASPECT_COLOR_BIT;
    outBarrier.subresourceRange.baseArrayLayer  = 0;
    outBarrier.subresourceRange.baseMipLevel    = 0;
    outBarrier.subresourceRange.levelCount      = VK_REMAINING_MIP_LEVELS;
    outBarrier.subresourceRange.baseArrayLayer  = 0;
    outBarrier.subresourceRange.layerCount      = VK_REMAINING_ARRAY_LAYERS;
}


} // /namespace LLGL



// ================================================================================
