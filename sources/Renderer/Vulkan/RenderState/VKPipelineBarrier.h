/*
 * VKPipelineBarrier.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_VK_PIPELINE_BARRIER_H
#define LLGL_VK_PIPELINE_BARRIER_H


#include <LLGL/Container/SmallVector.h>
#include "../VKPtr.h"
#include <vulkan/vulkan.h>
#include <memory>
#include <cstdint>


namespace LLGL
{


class Resource;

// Helper class to manage information for a Vulkan pipeline barrier command.
class VKPipelineBarrier
{

    public:

        // Returns true if this barrier is active in any stage.
        bool IsActive() const;

        // Submits this pipeline barrier into the specified command buffer.
        void Submit(VkCommandBuffer commandBuffer);

        // Emplaces the specified resource into the pipeline barrier.
        bool Emplace(std::uint32_t slot, Resource* resource, VkPipelineStageFlags stageFlags);

        // Removes the binding at the specified slot from the pipeline barrier.
        bool Remove(std::uint32_t slot);

        // Updates the internal barrier descritpors and return false if the barrier is no longer active.
        bool Update();

    private:

        struct ResourceBinding
        {
            std::uint32_t   slot        = 0;        // Unique binding slot
            Resource*       resource    = nullptr;
            long            stageFlags  = 0;
        };

    private:

        void InsertMemoryBarrier(VkPipelineStageFlags stageFlags, VkAccessFlags srcAccess, VkAccessFlags dstAccess);

        void InsertBufferMemoryBarrier(VkPipelineStageFlags stageFlags, VkAccessFlags srcAccess, VkAccessFlags dstAccess, VkBuffer buffer);

    private:

        VkPipelineStageFlags                    srcStageMask_   = 0;
        VkPipelineStageFlags                    dstStageMask_   = 0;
        SmallVector<ResourceBinding, 4u>        bindings_;
        SmallVector<VkMemoryBarrier, 1u>        memoryBarriers_;
        SmallVector<VkBufferMemoryBarrier, 1u>  bufferBarriers_;
        SmallVector<VkImageMemoryBarrier, 1u>   imageBarriers_;

};

using VKPipelineBarrierPtr = std::unique_ptr<VKPipelineBarrier>;


} // /namespace LLGL


#endif



// ================================================================================
