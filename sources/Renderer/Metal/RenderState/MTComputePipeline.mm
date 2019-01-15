/*
 * MTComputePipeline.mm
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "MTComputePipeline.h"
#include "../MTCore.h"
#include "../Shader/MTShaderProgram.h"
#include "../../CheckedCast.h"
#include <LLGL/ComputePipelineFlags.h>
#include <string>
#include <stdexcept>


namespace LLGL
{


MTComputePipeline::MTComputePipeline(id<MTLDevice> device, const ComputePipelineDescriptor& desc)
{
    /* Get native shader functions */
    auto shaderProgramMT = LLGL_CAST(const MTShaderProgram*, desc.shaderProgram);
    if (!shaderProgramMT)
        throw std::invalid_argument("failed to create compute pipeline due to missing shader program");

    id<MTLFunction> kernelFunc = shaderProgramMT->GetKernelMTLFunction();
    if (!kernelFunc)
        throw std::invalid_argument("failed to create compute pipeline due to missing compute shader in shader program");

    /* Create native compute pipeline state */
    NSError* error = nullptr;
    computePipelineState_ = [device newComputePipelineStateWithFunction:kernelFunc error:&error];
    if (!computePipelineState_)
        MTThrowIfCreateFailed(error, "MTLComputePipelineState");
}

void MTComputePipeline::Bind(id<MTLComputeCommandEncoder> computeEncoder)
{
    [computeEncoder setComputePipelineState:computePipelineState_];
}


} // /namespace LLGL



// ================================================================================
