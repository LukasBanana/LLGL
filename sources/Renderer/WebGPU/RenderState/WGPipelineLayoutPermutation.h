/*
 * WGPipelineLayoutPermutation.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_WG_PIPELINE_LAYOUT_PERMUTATION_H
#define LLGL_WG_PIPELINE_LAYOUT_PERMUTATION_H


#include <LLGL/Container/ArrayView.h>
#include <LLGL/Report.h>
#include <memory>
#include <cstdint>
#include <string>
#include <webgpu/webgpu.h>


namespace LLGL
{


struct WGResourceReflectionTable;

class WGPipelineLayoutPermutation
{

    public:

        WGPipelineLayoutPermutation(
            WGPUDevice                                  device,
            ArrayView<WGPUBindGroupLayoutEntry>         bindGroupEntries,
            ArrayView<std::string>                      bindGroupEntryNames,
            ArrayView<const WGResourceReflectionTable*> resourceTables,
            std::uint32_t                               immediateSize,
            const char*                                 debugName,
            Report&                                     outReport
        );
        ~WGPipelineLayoutPermutation();

        // Returns the native WebGPU pipeline layout object.
        inline WGPUPipelineLayout GetNative() const
        {
            return pipelineLayout_;
        }

    private:

        WGPUPipelineLayout  pipelineLayout_     = nullptr;
        WGPUBindGroupLayout bindGroupLayout_    = nullptr;

};

using WGPipelineLayoutPermutationSPtr = std::shared_ptr<WGPipelineLayoutPermutation>;


} // /namespace LLGL


#endif



// ================================================================================
