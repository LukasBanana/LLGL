/*
 * D3D11StagingBufferPool.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D11StagingBufferPool.h"
#include "../../../Core/CoreUtils.h"
#include <LLGL/Utils/ForRange.h>
#include <algorithm>


namespace LLGL
{


D3D11StagingBufferPool::D3D11StagingBufferPool(
    ID3D11Device*           device,
    ID3D11DeviceContext*    context,
    UINT                    chunkSize,
    D3D11_USAGE             usage,
    UINT                    cpuAccessFlags,
    UINT                    bindFlags)
:
    device_           { device               },
    context_          { context              },
    chunkSize_        { chunkSize            },
    usage_            { usage                },
    cpuAccessFlags_   { cpuAccessFlags       },
    bindFlags_        { bindFlags            },
    incrementOffsets_ { !NeedsUniqueBuffer() }
{
}

void D3D11StagingBufferPool::Reset()
{
    if (incrementOffsets_)
    {
        /* Reset offsets of all previously used chunks but skip tail of list */
        for_range(i, std::min<std::size_t>(chunkIdx_ + 1, chunks_.size()))
            chunks_[i].Reset();
    }
    chunkIdx_ = 0;
}

D3D11BufferRange D3D11StagingBufferPool::Write(const void* data, UINT dataSize, UINT alignment)
{
    const UINT alignedSize = GetAlignedSize(dataSize, alignment);

    /* Check if a new chunk must be allocated */
    if (chunkIdx_ == chunks_.size())
    {
        /* Allocate a new chunk */
        AllocChunk(alignedSize);
    }
    else if (!chunks_[chunkIdx_].Capacity(alignedSize))
    {
        /* Try to recycle next chunk or allocate new one */
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

    /* If a unique buffer is required with ever allocation, increment chunk index for next call */
    if (NeedsUniqueBuffer())
        ++chunkIdx_;

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
