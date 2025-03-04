/*
 * VKStagingBufferPool.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "VKStagingBufferPool.h"
#include "../Command/VKCommandContext.h"
#include "../Memory/VKDeviceMemoryManager.h"
#include "../../../Core/CoreUtils.h"
#include "../../../Core/Assertion.h"
#include <algorithm>


namespace LLGL
{


VKStagingBufferPool::VKStagingBufferPool(VKDeviceMemoryManager* deviceMemoryMngr, VkDeviceSize chunkSize) :
    deviceMemoryMngr_ { deviceMemoryMngr },
    chunkSize_        { chunkSize        }
{
}

void VKStagingBufferPool::InitializeDevice(VKDeviceMemoryManager* deviceMemoryMngr, VkDeviceSize chunkSize)
{
    deviceMemoryMngr_   = deviceMemoryMngr;
    chunkSize_          = chunkSize;
}

void VKStagingBufferPool::Reset()
{
    if (chunkIdx_ < chunks_.size())
        chunks_[chunkIdx_].Reset();
    chunkIdx_ = 0;
}

VkResult VKStagingBufferPool::WriteStaged(
    VkCommandBuffer commandBuffer,
    VkBuffer        dstBuffer,
    VkDeviceSize    dstOffset,
    const void*     data,
    VkDeviceSize    dataSize)
{
    /* Find a chunk that fits the requested data size or allocate a new chunk */
    while (chunkIdx_ < chunks_.size() && !chunks_[chunkIdx_].Capacity(dataSize))
    {
        chunks_[chunkIdx_].Reset();
        ++chunkIdx_;
    }

    if (chunkIdx_ == chunks_.size())
        AllocChunk(dataSize);

    /* Write data to current chunk */
    VKStagingBuffer& chunk = chunks_[chunkIdx_];
    return chunk.WriteAndIncrementOffset(deviceMemoryMngr_->GetVkDevice(), commandBuffer, dstBuffer, dstOffset, data, dataSize);
}


/*
 * ======= Private: =======
 */

void VKStagingBufferPool::AllocChunk(VkDeviceSize minChunkSize)
{
    LLGL_ASSERT_PTR(deviceMemoryMngr_);
    chunks_.emplace_back(*deviceMemoryMngr_, std::max(chunkSize_, minChunkSize));
    chunkIdx_ = chunks_.size() - 1;
}


} // /namespace LLGL



// ================================================================================
