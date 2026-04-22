/*
 * WGPipelineLayoutPermutation.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_WG_PIPELINE_LAYOUT_PERMUTATION_H
#define LLGL_WG_PIPELINE_LAYOUT_PERMUTATION_H


#include "WGBindGroupCache.h"
#include <LLGL/Container/ArrayView.h>
#include <LLGL/Report.h>
#include <memory>
#include <cstdint>
#include <string>
#include <vector>
#include <webgpu/webgpu.h>


namespace LLGL
{


struct WGCoreLimits;
struct WGResourceReflectionTable;

// Maps a descriptor from PipelineLayout to a WebGPU group binding.
struct WGPipelineLayoutDescriptor
{
    std::uint32_t groupIndex : 4; // WebGPU usually supports only 4-8 binding groups per render pipeline
    std::uint32_t entryIndex : 28;
};

struct WGPipelineLayoutGroup
{
    WGPUBindGroupLayout                     bindGroupLayout;
    std::vector<WGPUBindGroupLayoutEntry>   entries;
};

class WGPipelineLayoutPermutation
{

    public:

        WGPipelineLayoutPermutation(
            WGPUDevice                                  device,
            ArrayView<WGPUBindGroupLayoutEntry>         bindGroupEntries,
            ArrayView<std::string>                      bindGroupEntryNames,
            ArrayView<const WGResourceReflectionTable*> resourceTables,
            std::uint32_t                               immediateSize,
            const WGCoreLimits&                         coreLimits,
            const char*                                 debugName,
            Report&                                     outReport
        );
        ~WGPipelineLayoutPermutation();

        // Returns the native WebGPU pipeline layout object.
        inline WGPUPipelineLayout GetNative() const
        {
            return pipelineLayout_;
        }

        // Returns the layout bind groups.
        inline const std::vector<WGPipelineLayoutGroup>& GetLayoutGroups() const
        {
            return layoutGroups_;
        }

        // Returns the descriptor-to-bindgroup map.
        inline const std::vector<WGPipelineLayoutDescriptor>& GetDescriptorMap() const
        {
            return descriptorMap_;
        }

        // Returns the bind group cache. This will be used by the command buffer to emplace descriptors.
        inline WGBindGroupCache* GetBindGroupCache() const
        {
            return bindGroupCache_.get();
        }

    private:

        WGPUPipelineLayout                      pipelineLayout_ = nullptr;
        std::vector<WGPipelineLayoutGroup>      layoutGroups_;
        std::vector<WGPipelineLayoutDescriptor> descriptorMap_;
        WGBindGroupCachePtr                     bindGroupCache_;

};

using WGPipelineLayoutPermutationSPtr = std::shared_ptr<WGPipelineLayoutPermutation>;


} // /namespace LLGL


#endif



// ================================================================================
