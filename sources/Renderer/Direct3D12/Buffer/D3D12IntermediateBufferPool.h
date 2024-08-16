/*
 * D3D12IntermediateBufferPool.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D12_INTERMEDIATE_BUFFER_POOL_H
#define LLGL_D3D12_INTERMEDIATE_BUFFER_POOL_H


#include "D3D12StagingBuffer.h"
#include <d3d12.h>
#include <vector>


namespace LLGL
{


struct D3D12Resource;
class D3D12CommandContext;

/*
Pool to allocate buffers for intermediate copy operations.
These buffers may be reused immediately, i.e. their content won't persist until the next Reset() call; for this use D3D12StagingBufferPool.
*/
class D3D12IntermediateBufferPool
{

    public:

        D3D12IntermediateBufferPool() = default;
        D3D12IntermediateBufferPool(ID3D12Device* device, D3D12_HEAP_TYPE heapType = D3D12_HEAP_TYPE_DEFAULT);

        // Initializes the device object and chunk size.
        void InitializeDevice(ID3D12Device* device, D3D12_HEAP_TYPE heapType = D3D12_HEAP_TYPE_DEFAULT);

        // Resets all chunks in the pool.
        void Reset();

        // Allocates a buffer with a minimum of the specified size (plus padding for alignment).
        ID3D12Resource* AllocBuffer(UINT64 size, UINT alignment = 256u);

    private:

        ID3D12Device*                   device_     = nullptr;
        D3D12_HEAP_TYPE                 heapType_   = D3D12_HEAP_TYPE_DEFAULT;
        std::vector<D3D12StagingBuffer> chunks_; // Chunks are always growing in size, i.e. chunk[N] must always be smaller than chunk[N+1]

};


} // /namespace LLGL


#endif



// ================================================================================
