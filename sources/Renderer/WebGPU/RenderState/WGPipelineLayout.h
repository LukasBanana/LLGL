/*
 * WGPipelineLayout.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_WG_PIPELINE_LAYOUT_H
#define LLGL_WG_PIPELINE_LAYOUT_H


#include <LLGL/PipelineLayout.h>
#include <LLGL/PipelineLayoutFlags.h>
#include <LLGL/Report.h>
#include <LLGL/Container/ArrayView.h>
#include "WGPipelineLayoutPermutation.h"
#include <webgpu/webgpu.h>
#include <string>
#include <vector>


namespace LLGL
{


struct WGCoreLimits;
struct WGResourceReflectionTable;

class WGPipelineLayout final : public PipelineLayout
{

    public:

        #include <LLGL/Backend/PipelineLayout.inl>

    public:

        WGPipelineLayout(WGPUDevice device, const PipelineLayoutDescriptor& desc, const WGCoreLimits& coreLimits);

        // Creates a native WebGPU pipeline layout permutation using the specified resource reflection tables.
        WGPipelineLayoutPermutationSPtr CreatePermutation(
            WGPUDevice                                  device,
            ArrayView<const WGResourceReflectionTable*> resourceTables,
            Report&                                     outReport
        ) const;

    private:

        const WGCoreLimits&                     coreLimits_;

        std::uint32_t                           numHeapBindings_            = 0;
        std::uint32_t                           numBindings_                = 0;
        std::uint32_t                           numStaticSamplers_          = 0;
        std::uint32_t                           numUniforms_                = 0;

        std::string                             debugName_;
        std::uint32_t                           immediateSize_              = 0;
        std::vector<WGPUBindGroupLayoutEntry>   bindGroupEntriesTemplate_;
        std::vector<std::string>                bindingNames_;

};


} // /namespace LLGL


#endif



// ================================================================================
