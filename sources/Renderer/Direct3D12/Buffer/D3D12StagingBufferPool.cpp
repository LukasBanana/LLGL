/*
 * D3D12StagingBufferPool.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D12StagingBufferPool.h"
#include "../Command/D3D12CommandContext.h"
#include "../Command/D3D12CommandQueue.h"
#include "../D3D12Resource.h"
#include "../../../Core/CoreUtils.h"
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
    if (chunkIdx_ < chunks_.size())
        chunks_[chunkIdx_].Reset();
    chunkIdx_ = 0;
}

HRESULT D3D12StagingBufferPool::WriteStaged(
    D3D12CommandContext&    commandContext,
    D3D12Resource&          dstBuffer,
    UINT64                  dstOffset,
    const void*             data,
    UINT64                  dataSize)
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
    HRESULT hr;
    const D3D12_RESOURCE_STATES oldResourceState = dstBuffer.currentState;
    commandContext.TransitionResource(dstBuffer, D3D12_RESOURCE_STATE_COPY_DEST, true);
    {
        D3D12StagingBuffer& chunk = chunks_[chunkIdx_];
        hr = chunk.WriteAndIncrementOffset(commandContext.GetCommandList(), dstBuffer.Get(), dstOffset, data, dataSize);
    }
    commandContext.TransitionResource(dstBuffer, oldResourceState);
    return hr;
}

HRESULT D3D12StagingBufferPool::WriteImmediate(
    D3D12CommandContext&    commandContext,
    D3D12Resource&          dstBuffer,
    UINT64                  dstOffset,
    const void*             data,
    UINT64                  dataSize,
    UINT64                  alignment)
{
    /* Write data to global upload buffer and copy region to destination buffer */
    HRESULT hr;
    const D3D12_RESOURCE_STATES oldResourceState = dstBuffer.currentState;
    commandContext.TransitionResource(dstBuffer, D3D12_RESOURCE_STATE_COPY_DEST, true);
    {
        D3D12StagingBuffer& uploadBuffer = GetOrCreateCPUAccessBuffer(CPUAccessFlags::Write).GetUploadBufferAndGrow(dataSize, alignment);
        hr = uploadBuffer.Write(commandContext.GetCommandList(), dstBuffer.Get(), dstOffset, data, dataSize);
    }
    commandContext.TransitionResource(dstBuffer, oldResourceState);
    return hr;
}

HRESULT D3D12StagingBufferPool::ReadSubresourceRegion(
    D3D12CommandContext&    commandContext,
    D3D12CommandQueue&      commandQueue,
    D3D12Resource&          srcBuffer,
    UINT64                  srcOffset,
    void*                   data,
    UINT64                  dataSize,
    UINT64                  alignment)
{
    D3D12CPUAccessBuffer& cpuAccessBuffer = GetOrCreateCPUAccessBuffer(CPUAccessFlags::Read);
    return cpuAccessBuffer.ReadSubresourceRegion(
        commandContext,
        commandQueue,
        srcBuffer,
        srcOffset,
        data,
        dataSize,
        alignment
    );
}

D3D12StagingBufferPool::MapBufferTicket D3D12StagingBufferPool::MapFeedbackBuffer(
    D3D12CommandContext&    commandContext,
    D3D12CommandQueue&      commandQueue,
    D3D12Resource&          srcBuffer,
    const D3D12_RANGE&      readRange,
    void**                  mappedData)
{
    MapBufferTicket ticket;
    D3D12CPUAccessBuffer& cpuAccessBuffer = GetOrCreateCPUAccessBuffer(CPUAccessFlags::Read);
    HRESULT hr = cpuAccessBuffer.MapFeedbackBuffer(commandContext, commandQueue, srcBuffer, readRange, mappedData);
    if (SUCCEEDED(hr))
    {
        ticket.cpuAccessBuffer = &cpuAccessBuffer;
        ++numReadMappedCPUBuffers_;
    }
    ticket.hr = hr;
    return ticket;
}

void D3D12StagingBufferPool::UnmapFeedbackBuffer(MapBufferTicket ticket)
{
    if (D3D12CPUAccessBuffer* cpuAccessBuffer = ticket.cpuAccessBuffer)
    {
        cpuAccessBuffer->UnmapFeedbackBuffer();
        --numReadMappedCPUBuffers_;
    }
}

D3D12StagingBufferPool::MapBufferTicket D3D12StagingBufferPool::MapUploadBuffer(
    SIZE_T                  size,
    void**                  mappedData)
{
    MapBufferTicket ticket;
    D3D12CPUAccessBuffer& cpuAccessBuffer = GetOrCreateCPUAccessBuffer(CPUAccessFlags::Write);
    HRESULT hr = cpuAccessBuffer.MapUploadBuffer(size, mappedData);
    if (SUCCEEDED(hr))
    {
        ticket.cpuAccessBuffer = &cpuAccessBuffer;
        ++numWriteMappedCPUBuffers_;
    }
    ticket.hr = hr;
    return ticket;
}

void D3D12StagingBufferPool::UnmapUploadBuffer(
    D3D12CommandContext&    commandContext,
    D3D12CommandQueue&      commandQueue,
    D3D12Resource&          dstBuffer,
    const D3D12_RANGE&      writtenRange,
    MapBufferTicket         ticket)
{
    if (D3D12CPUAccessBuffer* cpuAccessBuffer = ticket.cpuAccessBuffer)
    {
        cpuAccessBuffer->UnmapUploadBuffer(commandContext, commandQueue, dstBuffer, writtenRange);
        --numWriteMappedCPUBuffers_;
    }
}


/*
 * ======= Private: =======
 */

void D3D12StagingBufferPool::AllocChunk(UINT64 minChunkSize)
{
    chunks_.emplace_back(device_, std::max(chunkSize_, minChunkSize));
    chunkIdx_ = chunks_.size() - 1;
}

D3D12CPUAccessBuffer& D3D12StagingBufferPool::GetOrCreateCPUAccessBuffer(long cpuAccessFlags)
{
    if (((cpuAccessFlags & CPUAccessFlags::Read ) == 0 || numReadMappedCPUBuffers_  < cpuAccessBuffers_.size()) &&
        ((cpuAccessFlags & CPUAccessFlags::Write) == 0 || numWriteMappedCPUBuffers_ < cpuAccessBuffers_.size()))
    {
        /* Try to find available CPU access buffer */
        for (D3D12CPUAccessBuffer& cpuAccessBuffer : cpuAccessBuffers_)
        {
            if ((cpuAccessBuffer.GetCurrentCPUAccessFlags() & cpuAccessFlags) == 0)
                return cpuAccessBuffer;
        }
    }

    /* If all CPU access buffers are already mapped, create a new one */
    cpuAccessBuffers_.emplace_back(device_);
    return cpuAccessBuffers_.back();
}


} // /namespace LLGL



// ================================================================================
