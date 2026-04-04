/*
 * WGComputePipeline.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_WG_COMPUTE_PIPELINE_H
#define LLGL_WG_COMPUTE_PIPELINE_H


#include "WGPipelineState.h"


namespace LLGL
{


class WGComputePipeline final : public WGPipelineState
{

    public:

        WGComputePipeline(const ComputePipelineDescriptor& desc);

        // Returns the native WebGPU compute pipeline object.
        inline WGPUComputePipeline GetNative() const
        {
            return computePipeline_;
        }

    private:

        WGPUComputePipeline computePipeline_ = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
