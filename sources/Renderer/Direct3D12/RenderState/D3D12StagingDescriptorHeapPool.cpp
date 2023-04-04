/*
 * D3D12StagingDescriptorHeapPool.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D12StagingDescriptorHeapPool.h"
#include "../D3D12Device.h"
#include "../../../Core/Assertion.h"
#include "../../../Core/CoreUtils.h"
#include <algorithm>


namespace LLGL
{


// Returns the maximum number of descriptor for the specified type
static UINT GetMaxDescriptorHeapSize(D3D12_DESCRIPTOR_HEAP_TYPE type)
{
    switch (type)
    {
        case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:    return 2048;
        default:                                    return UINT_MAX;
    }
}

// Returns the initial descriptor heap size for the specified type
static UINT GetInitialDescriptorHeapSize(D3D12_DESCRIPTOR_HEAP_TYPE type)
{
    switch (type)
    {
        case D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER:    return 128;
        default:                                    return 4096;
    }
}

D3D12StagingDescriptorHeapPool::D3D12StagingDescriptorHeapPool(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type)
{
    InitializeDevice(device, type);
}

void D3D12StagingDescriptorHeapPool::InitializeDevice(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE type)
{
    chunks_.clear();
    device_     = device;
    type_       = type;
    chunkSize_  = GetInitialDescriptorHeapSize(type);
    AllocChunk(chunkSize_);
}

void D3D12StagingDescriptorHeapPool::Reset()
{
    pendingOffset_ = 0;

    if (chunks_.size() > 1)
    {
        /* Try to consolidate chunks into a single one */
        UINT accumulatedSize = 0;
        for (const auto& chunk : chunks_)
            accumulatedSize += chunk.GetSize();

        if (accumulatedSize <= GetMaxDescriptorHeapSize(type_))
        {
            /* Create single chunk with accumulated size */
            chunks_.clear();
            AllocChunk(accumulatedSize);
        }
        else
            ResetChunks();
    }
    else
        ResetChunks();
}

D3D12_GPU_DESCRIPTOR_HANDLE D3D12StagingDescriptorHeapPool::CopyDescriptors(
    D3D12_CPU_DESCRIPTOR_HANDLE srcDescHandle,
    UINT                        firstDescriptor,
    UINT                        numDescriptors)
{
    LLGL_ASSERT_PTR(device_);

    /* Apply pending offset */
    IncrementOffset(pendingOffset_);

    /* Find a chunk that fits the requested data size or allocate a new chunk */
    while (chunkIdx_ < chunks_.size() && !chunks_[chunkIdx_].Capacity(firstDescriptor + numDescriptors))
        ++chunkIdx_;

    if (chunkIdx_ == chunks_.size())
        AllocChunk(numDescriptors);

    /* Copy descriptors into current chunk */
    chunks_[chunkIdx_].CopyDescriptors(device_, srcDescHandle, firstDescriptor, numDescriptors);

    /* Store pending offset to be applied with the next copy operation */
    pendingOffset_ = numDescriptors;

    return GetGpuHandleWithOffset();
}

ID3D12DescriptorHeap* D3D12StagingDescriptorHeapPool::GetDescriptorHeap() const
{
    return (chunkIdx_ < chunks_.size() ? chunks_.back().GetNative() : nullptr);
}

D3D12_GPU_DESCRIPTOR_HANDLE D3D12StagingDescriptorHeapPool::GetGpuHandleWithOffset() const
{
    if (chunkIdx_ < chunks_.size())
        return chunks_[chunkIdx_].GetGpuHandleWithOffset();
    else
        return {};
}

D3D12_CPU_DESCRIPTOR_HANDLE D3D12StagingDescriptorHeapPool::GetCpuHandleWithOffset(UINT descriptor) const
{
    if (chunkIdx_ < chunks_.size())
        return chunks_[chunkIdx_].GetCpuHandleWithOffset(descriptor);
    else
        return {};
}


/*
 * ======= Private: =======
 */

void D3D12StagingDescriptorHeapPool::AllocChunk(UINT minNumDescriptors)
{
    chunkSize_ = std::max(chunkSize_, minNumDescriptors);
    chunks_.emplace_back(device_, type_, chunkSize_);
    chunkIdx_ = chunks_.size() - 1;
}

void D3D12StagingDescriptorHeapPool::ResetChunks()
{
    /* Reset offsets of existing chunks */
    for (auto& chunk : chunks_)
        chunk.ResetOffset();
    chunkIdx_ = 0;
}

void D3D12StagingDescriptorHeapPool::IncrementOffset(UINT stride)
{
    if (chunkIdx_ < chunks_.size())
        chunks_[chunkIdx_].IncrementOffset(stride);
}


} // /namespace LLGL



// ================================================================================
