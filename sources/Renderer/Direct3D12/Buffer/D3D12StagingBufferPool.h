/*
 * D3D12StagingBufferPool.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D12_STAGING_BUFFER_POOL_H
#define LLGL_D3D12_STAGING_BUFFER_POOL_H


#include "D3D12StagingBuffer.h"
#include "D3D12CPUAccessBuffer.h"
#include <LLGL/RenderSystemFlags.h>
#include <vector>


namespace LLGL
{


struct D3D12Resource;
class D3D12CommandContext;
class D3D12CommandQueue;

class D3D12StagingBufferPool
{

    public:

        class MapBufferTicket
        {

                friend class D3D12StagingBufferPool;
                D3D12CPUAccessBuffer* cpuAccessBuffer = nullptr;

            public:

                HRESULT hr = S_OK;

        };

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

        MapBufferTicket MapFeedbackBuffer(
            D3D12CommandContext&    commandContext,
            D3D12CommandQueue&      commandQueue,
            D3D12Resource&          srcBuffer,
            const D3D12_RANGE&      readRange,
            void**                  mappedData
        );

        void UnmapFeedbackBuffer(MapBufferTicket ticket);

        MapBufferTicket MapUploadBuffer(
            SIZE_T                  size,
            void**                  mappedData
        );

        void UnmapUploadBuffer(
            D3D12CommandContext&    commandContext,
            D3D12CommandQueue&      commandQueue,
            D3D12Resource&          dstBuffer,
            const D3D12_RANGE&      writtenRange,
            MapBufferTicket         ticket
        );

    private:

        // Allocates a new chunk with the specified minimal size.
        void AllocChunk(UINT64 minChunkSize);

        // Returns the first available CPU access buffer or creates a new one.
        D3D12CPUAccessBuffer& GetOrCreateCPUAccessBuffer(long cpuAccessFlags);

    private:

        ID3D12Device*                       device_                     = nullptr;

        std::vector<D3D12StagingBuffer>     chunks_;
        std::size_t                         chunkIdx_                   = 0;
        UINT64                              chunkSize_                  = 0;

        std::vector<D3D12CPUAccessBuffer>   cpuAccessBuffers_;
        std::size_t                         numReadMappedCPUBuffers_    = 0;
        std::size_t                         numWriteMappedCPUBuffers_   = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
