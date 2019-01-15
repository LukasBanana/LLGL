/*
 * MTComputePipeline.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_MT_COMPUTE_PIPELINE_H
#define LLGL_MT_COMPUTE_PIPELINE_H


#import <Metal/Metal.h>

#include <LLGL/ComputePipeline.h>
#include <LLGL/ForwardDecls.h>
#include <cstdint>


namespace LLGL
{


class MTComputePipeline : public ComputePipeline
{

    public:

        MTComputePipeline(id<MTLDevice> device, const ComputePipelineDescriptor& desc);

        // Binds the compute pipeline state with the specified command encoder.
        void Bind(id<MTLComputeCommandEncoder> computeEncoder);

    private:

        id<MTLComputePipelineState> computePipelineState_ = nil;

};


} // /namespace LLGL


#endif



// ================================================================================
