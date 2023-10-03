/*
 * D3D11StagingBufferPool.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D11StagingBufferPool.h"
#include "../../../Core/CoreUtils.h"


namespace LLGL
{


// Returns true if the specified D3D context requires separate segments for staging buffers
static bool IsSegmentedStagingRequired(ID3D11DeviceContext* context)
{
    #if 0//TODO: Not sure if this is needed at the moment
    //return (context->GetType() == D3D11_DEVICE_CONTEXT_DEFERRED);
    #else
    return false;
    #endif
}

D3D11StagingBufferPool::D3D11StagingBufferPool(
    ID3D11Device*           device,
    ID3D11DeviceContext*    context,
    UINT                    chunkSize,
    D3D11_USAGE             usage,
    UINT                    cpuAccessFlags,
    UINT                    bindFlags)
:
    device_           { device                              },
    context_          { context                             },
    chunkSize_        { chunkSize                           },
    usage_            { usage                               },
    cpuAccessFlags_   { cpuAccessFlags                      },
    bindFlags_        { bindFlags                           },
    incrementOffsets_ { IsSegmentedStagingRequired(context) }
{
}

void D3D11StagingBufferPool::Reset()
{
    for (D3D11StagingBuffer& chunk : chunks_)
        chunk.Reset();
    chunkIdx_ = 0;
}

D3D11BufferRange D3D11StagingBufferPool::Write(const void* data, UINT dataSize, UINT alignment)
{
    const UINT alignedSize = GetAlignedSize(dataSize, alignment);

    /* Check if a new chunk must be allocated */
    if (chunkIdx_ == chunks_.size())
        AllocChunk(alignedSize);
    else if (!chunks_[chunkIdx_].Capacity(alignedSize))
    {
        ++chunkIdx_;
        if (chunkIdx_ == chunks_.size())
            AllocChunk(alignedSize);
    }

    /* Write data to current chunk */
    D3D11StagingBuffer& chunk = chunks_[chunkIdx_];
    D3D11BufferRange range = { chunk.GetNative(), chunk.GetOffset(), alignedSize };
    {
        if (incrementOffsets_)
            chunk.WriteAndIncrementOffset(context_, data, dataSize, alignedSize);
        else
            chunk.Write(context_, data, dataSize);
    }
    return range;
}


/*
 * ======= Private: =======
 */

void D3D11StagingBufferPool::AllocChunk(UINT minChunkSize)
{
    chunks_.emplace_back(device_, std::max(chunkSize_, minChunkSize), usage_, cpuAccessFlags_, bindFlags_);
    chunkIdx_ = chunks_.size() - 1;
}


} // /namespace LLGL



// ================================================================================
