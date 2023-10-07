/*
 * D3D12PipelineCache.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D12PipelineCache.h"


namespace LLGL
{


D3D12PipelineCache::D3D12PipelineCache(const Blob& initialBlob) :
    initialBlob_ { initialBlob ? Blob::CreateCopy(initialBlob.GetData(), initialBlob.GetSize()) : Blob{} }
{
}

Blob D3D12PipelineCache::GetBlob() const
{
    /* Prefer native blob in case it has been updated after an initial blob was provided */
    if (nativeBlob_)
        return Blob::CreateCopy(nativeBlob_->GetBufferPointer(), nativeBlob_->GetBufferSize());
    else if (initialBlob_)
        return Blob::CreateCopy(initialBlob_.GetData(), initialBlob_.GetSize());
    else
        return Blob{};
}

D3D12_CACHED_PIPELINE_STATE D3D12PipelineCache::GetCachedPSO() const
{
    /* Prefer native blob in case it has been updated after an initial blob was provided */
    D3D12_CACHED_PIPELINE_STATE cachedState = {};
    if (nativeBlob_)
    {
        cachedState.pCachedBlob             = nativeBlob_->GetBufferPointer();
        cachedState.CachedBlobSizeInBytes   = nativeBlob_->GetBufferSize();
    }
    else if (initialBlob_)
    {
        cachedState.pCachedBlob             = initialBlob_.GetData();
        cachedState.CachedBlobSizeInBytes   = initialBlob_.GetSize();
    }
    return cachedState;
}


} // /namespace LLGL



// ================================================================================
