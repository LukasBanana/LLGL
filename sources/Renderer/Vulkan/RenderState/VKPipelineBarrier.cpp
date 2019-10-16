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

static VkPipelineStageFlags ToVkStageFlags(long stageFlags)
{
    VkPipelineStageFlags bitmask = 0;

    if ((stageFlags & StageFlags::VertexStage) != 0)
        bitmask |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
    if ((stageFlags & StageFlags::TessControlStage) != 0)
        bitmask |= VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT;
    if ((stageFlags & StageFlags::TessEvaluationStage) != 0)
        bitmask |= VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;
    if ((stageFlags & StageFlags::GeometryStage) != 0)
        bitmask |= VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
    if ((stageFlags & StageFlags::FragmentStage) != 0)
        bitmask |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    if ((stageFlags & StageFlags::ComputeStage) != 0)
        bitmask |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

    return bitmask;
}

void VKPipelineBarrier::InsertMemoryBarrier(long stageFlags, VkAccessFlags srcAccess, VkAccessFlags dstAccess)
{
    auto stagesBitmask = ToVkStageFlags(stageFlags);

    srcStageMask_ |= stagesBitmask;
    dstStageMask_ |= stagesBitmask;

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
