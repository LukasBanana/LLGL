/*
 * VKPipelineBarrier.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_VK_PIPELINE_BARRIER_H
#define LLGL_VK_PIPELINE_BARRIER_H


#include <vulkan/vulkan.h>
#include "../VKPtr.h"
#include <vector>


namespace LLGL
{


// Helper class to manage information for a Vulkan pipeline barrier command.
class VKPipelineBarrier
{

    public:

        // Returns true if this barrier is enabled.
        bool IsEnabled() const;

        // Submits this pipeline barrier into the specified command buffer.
        void Submit(VkCommandBuffer commandBuffer);

        // Inserts a memory barrier
        void InsertMemoryBarrier(long stageFlags, VkAccessFlags srcAccess, VkAccessFlags dstAccess);

    private:

        VkPipelineStageFlags                srcStageMask_   = 0;
        VkPipelineStageFlags                dstStageMask_   = 0;
        std::vector<VkMemoryBarrier>        memoryBarrier_;
        std::vector<VkBufferMemoryBarrier>  bufferBarriers_;
        std::vector<VkImageMemoryBarrier>   imageBarriers_;

};


} // /namespace LLGL


#endif



// ================================================================================
