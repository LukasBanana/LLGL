/*
 * D3D12StagingBufferPool.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12StagingBufferPool.h"
#include "../Command/D3D12CommandContext.h"
#include "../D3D12Resource.h"
#include <algorithm>


namespace LLGL
{


D3D12StagingBufferPool::D3D12StagingBufferPool(ID3D12Device* device, UINT64 chunkSize) :
    device_    { device    },
    chunkSize_ { chunkSize }
{
}

void D3D12StagingBufferPool::Reset()
{
    for (auto& chunk : chunks_)
        chunk.Reset();
    chunkIdx_ = 0;
}

void D3D12StagingBufferPool::Write(
    D3D12CommandContext&    commandContext,
    D3D12Resource&          dstBuffer,
    UINT64                  dstOffset,
    const void*             data,
    UINT64                  dataSize)
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
    commandContext.TransitionResource(dstBuffer, D3D12_RESOURCE_STATE_COPY_DEST);
    {
        auto& chunk = chunks_[chunkIdx_];
        chunk.Write(device_, commandContext.GetCommandList(), dstBuffer.Get(), dstOffset, data, dataSize);
    }
    commandContext.TransitionResource(dstBuffer, dstBuffer.usageState);
}


/*
 * ======= Private: =======
 */

void D3D12StagingBufferPool::AllocChunk(UINT64 minChunkSize)
{
    chunks_.emplace_back(device_, std::max(chunkSize_, minChunkSize));
    chunkIdx_ = chunks_.size() - 1;
}


} // /namespace LLGL



// ================================================================================
