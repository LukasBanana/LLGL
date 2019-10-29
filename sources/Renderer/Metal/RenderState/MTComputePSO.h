/*
 * MTComputePSO.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_MT_COMPUTE_PSO_H
#define LLGL_MT_COMPUTE_PSO_H


#include "MTPipelineState.h"


namespace LLGL
{


struct ComputePipelineDescriptor;
class MTShaderProgram;

class MTComputePSO final : public MTPipelineState
{

    public:

        MTComputePSO(id<MTLDevice> device, const ComputePipelineDescriptor& desc);

        // Binds the compute pipeline state with the specified command encoder.
        void Bind(id<MTLComputeCommandEncoder> computeEncoder);

        // Returns the shader program this pipeline was created with.
        inline const MTShaderProgram* GetShaderProgram() const
        {
            return shaderProgram_;
        }

    private:

        id<MTLComputePipelineState> computePipelineState_   = nil;
        const MTShaderProgram*      shaderProgram_          = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
