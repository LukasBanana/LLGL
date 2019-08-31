/*
 * D3D12StagingBufferPool.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
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

class D3D12StagingBufferPool
{

    public:

        D3D12StagingBufferPool(ID3D12Device* device, UINT64 chunkSize);

        // Resets all chunks in the pool.
        void Reset();

        // Writes the specified data to the destination buffer.
        void Write(
            D3D12CommandContext&    commandContext,
            D3D12Resource&          dstBuffer,
            UINT64                  dstOffset,
            const void*             data,
            UINT64                  dataSize
        );

    private:

        // Allocates a new chunk with the specified minimal size.
        void AllocChunk(UINT64 minChunkSize);

    private:

        ID3D12Device*                   device_     = nullptr;
        std::vector<D3D12StagingBuffer> chunks_;
        std::size_t                     chunkIdx_   = 0;
        UINT64                          chunkSize_  = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
