/*
 * D3D12IntermediateBufferPool.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D12IntermediateBufferPool.h"
#include "../D3D12Resource.h"
#include "../../../Core/CoreUtils.h"
#include <algorithm>


namespace LLGL
{


D3D12IntermediateBufferPool::D3D12IntermediateBufferPool(ID3D12Device* device, D3D12_HEAP_TYPE heapType) :
    device_   { device   },
    heapType_ { heapType }
{
}

void D3D12IntermediateBufferPool::InitializeDevice(ID3D12Device* device, D3D12_HEAP_TYPE heapType)
{
    device_ = device;
    heapType_ = heapType;
}

void D3D12IntermediateBufferPool::Reset()
{
    /* Release all buffers but the largest one */
    if (!chunks_.empty())
    {
        D3D12StagingBuffer largestChunk = std::move(chunks_.back());
        chunks_.clear();
        chunks_.push_back(std::move(largestChunk));
    }
}

ID3D12Resource* D3D12IntermediateBufferPool::AllocBuffer(UINT64 size, UINT alignment)
{
    if (chunks_.empty() || chunks_.back().GetSize() < size)
    {
        /* Allocate new and larger buffer with 150% growth strategy */
        const UINT64 capacity = GetAlignedSize<UINT64>(size + size/2, alignment);
        chunks_.emplace_back(device_, capacity, alignment, heapType_);
    }
    return chunks_.back().GetNative();
}


} // /namespace LLGL



// ================================================================================
