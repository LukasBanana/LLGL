/*
 * D3D11IntermediateBufferPool.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_D3D11_INTERMEDIATE_BUFFER_POOL_H
#define LLGL_D3D11_INTERMEDIATE_BUFFER_POOL_H


#include "D3D11IntermediateBuffer.h"
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

class D3D11IntermediateBufferPool
{

    public:

        D3D11IntermediateBufferPool(
            ID3D11Device*           device,
            ID3D11DeviceContext*    context,
            UINT                    chunkSize,
            UINT                    bindFlags   = 0,
            UINT                    miscFlags   = 0
        );

        // Resets all chunks in the pool.
        void Reset();

        // Writes the specified data to the destination buffer using the staging pool.
        D3D11BufferRange Write(const void* data, UINT dataSize);

    private:

        // Allocates a new chunk with the specified minimal size.
        void AllocChunk(UINT minChunkSize);

    private:

        ID3D11Device*                           device_             = nullptr;
        ID3D11DeviceContext*                    context_            = nullptr;

        std::vector<D3D11IntermediateBuffer>    chunks_;
        std::size_t                             chunkIdx_           = 0;
        UINT                                    chunkSize_          = 0;
        UINT                                    bindFlags_          = 0;
        UINT                                    miscFlags_          = 0;
        bool                                    incrementOffsets_   = false;

};


} // /namespace LLGL


#endif



// ================================================================================
