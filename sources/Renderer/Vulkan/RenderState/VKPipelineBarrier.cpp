/*
 * VKPipelineBarrier.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKPipelineBarrier.h"
#include <LLGL/ShaderFlags.h>


namespace LLGL
{


bool VKPipelineBarrier::IsEnabled() const
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
        static_cast<std::uint32_t>(memoryBarrier_.size()),
        memoryBarrier_.data(),
        static_cast<std::uint32_t>(bufferBarriers_.size()),
        bufferBarriers_.data(),
        static_cast<std::uint32_t>(imageBarriers_.size()),
        imageBarriers_.data()
    );
}

void VKPipelineBarrier::InsertMemoryBarrier(VkPipelineStageFlags stageFlags, VkAccessFlags srcAccess, VkAccessFlags dstAccess)
{
    srcStageMask_ |= stageFlags;
    dstStageMask_ |= stageFlags;

    /* Check if a memory barrier alread exists */
    for (const auto& barrier : memoryBarrier_)
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
    memoryBarrier_.push_back(barrier);
}


} // /namespace LLGL



// ================================================================================
