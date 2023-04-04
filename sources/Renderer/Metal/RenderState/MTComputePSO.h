/*
 * MTComputePSO.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_MT_COMPUTE_PSO_H
#define LLGL_MT_COMPUTE_PSO_H


#include "MTPipelineState.h"


namespace LLGL
{


struct ComputePipelineDescriptor;
class MTShader;

class MTComputePSO final : public MTPipelineState
{

    public:

        MTComputePSO(id<MTLDevice> device, const ComputePipelineDescriptor& desc);

        // Binds the compute pipeline state with the specified command encoder.
        void Bind(id<MTLComputeCommandEncoder> computeEncoder);

        // Returns the compute shader this pipeline was created with.
        inline const MTShader* GetComputeShader() const
        {
            return computeShader_;
        }

    private:

        id<MTLComputePipelineState> CreateNativeComputePipelineState(
            id<MTLDevice>   device,
            id<MTLFunction> function,
            NSError*&       error
        );

    private:

        id<MTLComputePipelineState> computePipelineState_   = nil;
        const MTShader*             computeShader_          = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
