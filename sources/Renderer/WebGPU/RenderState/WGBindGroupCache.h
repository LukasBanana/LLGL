/*
 * WGBindGroupCache.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_WG_BIND_GROUP_CACHE_H
#define LLGL_WG_BIND_GROUP_CACHE_H


#include "WGBindGroup.h"
#include <LLGL/ForwardDecls.h>
#include <LLGL/Container/SmallVector.h>
#include <unordered_map>
#include <memory>
#include <webgpu/webgpu.h>


namespace LLGL
{


class WGPipelineLayoutPermutation;

// One bind group cache per pipeline layout permutation, since each WGPUBindGroup has entries that are tied to a WGPUBindGroupLayout.
class WGBindGroupCache
{

    public:

        WGBindGroupCache(const WGPipelineLayoutPermutation* pipelineLayoutPermutation);

        // Emplaces the specified resource descriptor into the cache. Returns true if the cache was invalidated.
        bool EmplaceResource(Resource& resource, std::uint32_t location);

        // Binds all invalidated bind groups to the specified render pass encoder.
        void FlushRenderPassBindGroups(WGPUDevice device, WGPURenderPassEncoder renderPassEncoder);

        // Binds all invalidated bind groups to the specified compute pass encoder.
        void FlushComputePassBindGroups(WGPUDevice device, WGPUComputePassEncoder computePassEncoder);

        // Invalidates the cache, forcing a full binding of all groups on the next flush.
        void Invalidate(std::uint16_t first = 0, std::uint16_t last = 0xFFFF);

    private:

        using BindGroupKey = std::size_t;
        using BindGroupMap = std::unordered_map<BindGroupKey, WGBindGroupPtr>;

        struct BindGroupInfo
        {
            std::uint32_t dirtyBit   :  1; // If 1, this bind group must be updated in FlushBindGroups().
            std::uint32_t firstEntry : 15;
            std::uint32_t numEntries : 16;
        };

    private:

        void ResetDirtRange();

        void FlushRenderPassBindGroup(WGPUDevice device, WGPURenderPassEncoder renderPassEncoder, std::uint32_t groupIndex);
        void SetRenderPassBindGroup(WGPURenderPassEncoder renderPassEncoder, std::uint32_t groupIndex, const WGBindGroup* bindGroup);

        void InvalidateBindGroup(std::uint32_t groupIndex);

    private:

        const WGPipelineLayoutPermutation*  pipelineLayoutPermutation_  = nullptr;
        SmallVector<BindGroupInfo, 2>       bindGroups_;
        std::uint16_t                       bindGroupDirtyRange_[2]     = { 0, 0 };
        std::vector<WGPUBindGroupEntry>     bindGroupEntries_; // Same number of elements as WGPipelineLayoutPermutation::GetDescriptorMap()

        BindGroupMap                        cachedBindGroups_;

};

using WGBindGroupCachePtr = std::unique_ptr<WGBindGroupCache>;


} // /namespace LLGL


#endif



// ================================================================================
