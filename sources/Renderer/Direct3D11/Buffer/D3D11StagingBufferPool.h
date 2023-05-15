/*
 * D3D11StagingBufferPool.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D11_STAGING_BUFFER_POOL_H
#define LLGL_D3D11_STAGING_BUFFER_POOL_H


#include "D3D11StagingBuffer.h"
#include <d3d12.h>
#include <vector>


namespace LLGL
{


// Helper structure to simplify writing to the pool of intermediate buffers.
struct D3D11BufferRange
{
    ID3D11Buffer*   native;
    UINT            offset;
    UINT            size;
};

class D3D11StagingBufferPool
{

    public:

        D3D11StagingBufferPool(
            ID3D11Device*           device,
            ID3D11DeviceContext*    context,
            UINT                    chunkSize,
            D3D11_USAGE             usage           = D3D11_USAGE_STAGING,
            UINT                    cpuAccessFlags  = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ,
            UINT                    bindFlags       = 0
        );

        // Resets all chunks in the pool.
        void Reset();

        // Writes the specified data to the destination buffer using the staging pool.
        D3D11BufferRange Write(const void* data, UINT dataSize, UINT alignment = 0);

    private:

        // Allocates a new chunk with the specified minimal size.
        void AllocChunk(UINT minChunkSize);

    private:

        ID3D11Device*                   device_             = nullptr;
        ID3D11DeviceContext*            context_            = nullptr;

        std::vector<D3D11StagingBuffer> chunks_;
        std::size_t                     chunkIdx_           = 0;
        UINT                            chunkSize_          = 0;
        D3D11_USAGE                     usage_              = D3D11_USAGE_STAGING;
        UINT                            cpuAccessFlags_     = D3D11_CPU_ACCESS_WRITE | D3D11_CPU_ACCESS_READ;
        UINT                            bindFlags_          = 0;
        bool                            incrementOffsets_   = false;

};


} // /namespace LLGL


#endif



// ================================================================================
