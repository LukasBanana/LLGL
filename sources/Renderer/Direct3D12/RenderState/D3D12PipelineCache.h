/*
 * D3D12PipelineCache.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D12_PIPELINE_CACHE_H
#define LLGL_D3D12_PIPELINE_CACHE_H


#include <LLGL/PipelineCache.h>
#include <d3d12.h>
#include "../../DXCommon/ComPtr.h"


namespace LLGL
{


class D3D12PipelineCache final : public PipelineCache
{

    public:

        D3D12PipelineCache() = default;

        // Initializes the pipeline cache with the specified initial blob.
        D3D12PipelineCache(const Blob& initialBlob);

        Blob GetBlob() const override;

    public:

        inline void SetNativeBlob(const ComPtr<ID3DBlob>& blob)
        {
            nativeBlob_ = blob;
        }

        inline void SetNativeBlob(ComPtr<ID3DBlob>&& blob)
        {
            nativeBlob_ = std::forward<ComPtr<ID3DBlob>>(blob);
        }

        // Returns true if this pipeline cache has an initial blob.
        inline bool HasInitialBlob() const
        {
            return bool(initialBlob_);
        }

        // Returns the cached PSO blob.
        D3D12_CACHED_PIPELINE_STATE GetCachedPSO() const;

    private:

        Blob                initialBlob_;
        ComPtr<ID3DBlob>    nativeBlob_;

};


} // /namespace LLGL


#endif



// ================================================================================
