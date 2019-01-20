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


class MTShaderProgram;

class MTComputePipeline : public ComputePipeline
{

    public:

        MTComputePipeline(id<MTLDevice> device, const ComputePipelineDescriptor& desc);

        // Binds the compute pipeline state with the specified command encoder.
        void Bind(id<MTLComputeCommandEncoder> computeEncoder);

    public:

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
