/*
 * MTStagingBufferPool.mm
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "MTStagingBufferPool.h"
#include <algorithm>


namespace LLGL
{


MTStagingBufferPool::MTStagingBufferPool(id<MTLDevice> device, NSUInteger chunkSize) :
    device_    { device    },
    chunkSize_ { chunkSize }
{
}

void MTStagingBufferPool::Reset()
{
    for (auto& chunk : chunks_)
        chunk.Reset();
    chunkIdx_ = 0;
}

void MTStagingBufferPool::Write(
    const void*     data,
    NSUInteger      dataSize,
    id<MTLBuffer>&  srcBuffer,
    NSUInteger&     srcOffset)
{
    /* Check if a new chunk must be allocated */
    if (chunkIdx_ == chunks_.size())
        AllocChunk(dataSize);
    else if (!chunks_[chunkIdx_].Capacity(dataSize))
    {
        ++chunkIdx_;
        if (chunkIdx_ == chunks_.size())
            AllocChunk(dataSize);
    }

    /* Write data to current chunk */
    auto& chunk = chunks_[chunkIdx_];
    srcOffset = chunk.GetOffset();
    srcBuffer = chunk.GetNative();
    chunk.Write(data, dataSize);
}


/*
 * ======= Private: =======
 */

void MTStagingBufferPool::AllocChunk(NSUInteger minChunkSize)
{
    chunks_.emplace_back(device_, std::max(chunkSize_, minChunkSize));
    chunkIdx_ = chunks_.size() - 1;
}


} // /namespace LLGL



// ================================================================================
