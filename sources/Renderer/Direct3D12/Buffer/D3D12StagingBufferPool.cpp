/*
 * D3D12StagingBufferPool.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12StagingBufferPool.h"
#include "../Command/D3D12CommandContext.h"
#include "../D3D12Resource.h"
#include "../../../Core/Helper.h"
#include <algorithm>


namespace LLGL
{


D3D12StagingBufferPool::D3D12StagingBufferPool(ID3D12Device* device, UINT64 chunkSize) :
    device_    { device    },
    chunkSize_ { chunkSize }
{
}

void D3D12StagingBufferPool::InitializeDevice(ID3D12Device* device, UINT64 chunkSize)
{
    device_    = device;
    chunkSize_ = chunkSize;
}

void D3D12StagingBufferPool::Reset()
{
    for (auto& chunk : chunks_)
        chunk.Reset();
    chunkIdx_ = 0;
}

void D3D12StagingBufferPool::WriteStaged(
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
    commandContext.TransitionResource(dstBuffer, D3D12_RESOURCE_STATE_COPY_DEST, true);
    {
        auto& chunk = chunks_[chunkIdx_];
        chunk.WriteAndIncrementOffset(commandContext.GetCommandList(), dstBuffer.Get(), dstOffset, data, dataSize);
    }
    commandContext.TransitionResource(dstBuffer, dstBuffer.usageState, true);
}

void D3D12StagingBufferPool::WriteImmediate(
    D3D12CommandContext&    commandContext,
    D3D12Resource&          dstBuffer,
    UINT64                  dstOffset,
    const void*             data,
    UINT64                  dataSize,
    UINT64                  alignment)
{
    /* Check if global upload buffer must be resized */
    UINT64 alignedSize = GetAlignedSize(dataSize, alignment);
    if (!globalUploadBuffer_.Capacity(alignedSize))
    {
        /* Use at least a bigger alignment for allocating the global upload buffer to reduce number of reallocations */
        static const UINT64 g_minAlignment = 4096ull;
        if (alignment < g_minAlignment)
            alignedSize = GetAlignedSize(alignedSize, g_minAlignment);

        /* Resize global upload buffer */
        globalUploadBuffer_.Create(device_, alignedSize);
    }

    /* Write data to global upload buffer and copy region to destination buffer */
    commandContext.TransitionResource(dstBuffer, D3D12_RESOURCE_STATE_COPY_DEST, true);
    {
        globalUploadBuffer_.Write(commandContext.GetCommandList(), dstBuffer.Get(), dstOffset, data, dataSize);
    }
    commandContext.TransitionResource(dstBuffer, dstBuffer.usageState, true);
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
