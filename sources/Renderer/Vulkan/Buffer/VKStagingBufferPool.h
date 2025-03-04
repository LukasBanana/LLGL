/*
 * VKStagingBufferPool.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_VK_STAGING_BUFFER_POOL_H
#define LLGL_VK_STAGING_BUFFER_POOL_H


#include "VKStagingBuffer.h"
#include <LLGL/RenderSystemFlags.h>
#include <vector>


namespace LLGL
{


class VKCommandContext;

class VKStagingBufferPool
{

    public:

        VKStagingBufferPool() = default;
        VKStagingBufferPool(VKDeviceMemoryManager* deviceMemoryMngr, VkDeviceSize chunkSize);

        // Initializes the device object and chunk size.
        void InitializeDevice(VKDeviceMemoryManager* deviceMemoryMngr, VkDeviceSize chunkSize);

        // Resets all chunks in the pool.
        void Reset();

        // Writes the specified data to the destination buffer using the staging pool.
        VkResult WriteStaged(
            VkCommandBuffer commandBuffer,
            VkBuffer        dstBuffer,
            VkDeviceSize    dstOffset,
            const void*     data,
            VkDeviceSize    dataSize
        );

    private:

        // Allocates a new chunk with the specified minimal size.
        void AllocChunk(VkDeviceSize minChunkSize);

    private:

        VKDeviceMemoryManager*          deviceMemoryMngr_   = nullptr;

        std::vector<VKStagingBuffer>    chunks_;
        std::size_t                     chunkIdx_           = 0;
        VkDeviceSize                    chunkSize_          = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
