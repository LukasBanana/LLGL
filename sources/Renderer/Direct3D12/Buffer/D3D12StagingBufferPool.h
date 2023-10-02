/*
 * D3D12StagingBufferPool.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D12_STAGING_BUFFER_POOL_H
#define LLGL_D3D12_STAGING_BUFFER_POOL_H


#include "D3D12StagingBuffer.h"
#include <d3d12.h>
#include <vector>


namespace LLGL
{


struct D3D12Resource;
class D3D12CommandContext;
class D3D12CommandQueue;

class D3D12StagingBufferPool
{

    public:

        D3D12StagingBufferPool() = default;
        D3D12StagingBufferPool(ID3D12Device* device, UINT64 chunkSize);

        // Initializes the device object and chunk size.
        void InitializeDevice(ID3D12Device* device, UINT64 chunkSize);

        // Resets all chunks in the pool.
        void Reset();

        // Writes the specified data to the destination buffer using the staging pool.
        HRESULT WriteStaged(
            D3D12CommandContext&    commandContext,
            D3D12Resource&          dstBuffer,
            UINT64                  dstOffset,
            const void*             data,
            UINT64                  dataSize
        );

        // Writes the specified data to the destination buffer using the global upload buffer.
        HRESULT WriteImmediate(
            D3D12CommandContext&    commandContext,
            D3D12Resource&          dstBuffer,
            UINT64                  dstOffset,
            const void*             data,
            UINT64                  dataSize,
            UINT64                  alignment   = 256u
        );

        // Copies the specified subresource region into the global readback buffer and writes it into the output data.
        HRESULT ReadSubresourceRegion(
            D3D12CommandContext&    commandContext,
            D3D12CommandQueue&      commandQueue,
            D3D12Resource&          srcBuffer,
            UINT64                  srcOffset,
            void*                   data,
            UINT64                  dataSize,
            UINT64                  alignment   = 256u
        );

    private:

        // Allocates a new chunk with the specified minimal size.
        void AllocChunk(UINT64 minChunkSize);

        // Resizes the specified staging buffer, but only grows its size.
        void ResizeBuffer(
            D3D12StagingBuffer& stagingBuffer,
            D3D12_HEAP_TYPE     heapType,
            UINT64              size,
            UINT64              alignment
        );

        D3D12StagingBuffer& GetUploadBufferAndGrow(UINT64 size, UINT64 alignment);
        D3D12StagingBuffer& GetReadbackBufferAndGrow(UINT64 size, UINT64 alignment);

    private:

        ID3D12Device*                   device_             = nullptr;

        std::vector<D3D12StagingBuffer> chunks_;
        std::size_t                     chunkIdx_           = 0;
        UINT64                          chunkSize_          = 0;

        D3D12StagingBuffer              globalUploadBuffer_;
        D3D12StagingBuffer              globalReadbackBuffer_;

};


} // /namespace LLGL


#endif



// ================================================================================
