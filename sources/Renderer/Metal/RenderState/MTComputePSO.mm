/*
 * MTComputePSO.mm
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "MTComputePSO.h"
#include "../MTCore.h"
#include "../Shader/MTShaderProgram.h"
#include "../../CheckedCast.h"
#include <LLGL/PipelineStateFlags.h>
#include <string>
#include <stdexcept>


namespace LLGL
{


MTComputePSO::MTComputePSO(id<MTLDevice> device, const ComputePipelineDescriptor& desc) :
    MTPipelineState { false }
{
    /* Get native shader functions */
    shaderProgram_ = LLGL_CAST(const MTShaderProgram*, desc.shaderProgram);
    if (!shaderProgram_)
        throw std::invalid_argument("failed to create compute pipeline state due to missing shader program");

    id<MTLFunction> kernelFunc = shaderProgram_->GetKernelMTLFunction();
    if (!kernelFunc)
        throw std::invalid_argument("failed to create compute pipeline due to missing compute shader in shader program");

    /* Create native compute pipeline state */
    NSError* error = nullptr;
    computePipelineState_ = [device newComputePipelineStateWithFunction:kernelFunc error:&error];
    if (!computePipelineState_)
        MTThrowIfCreateFailed(error, "MTLComputePipelineState");
}

void MTComputePSO::Bind(id<MTLComputeCommandEncoder> computeEncoder)
{
    [computeEncoder setComputePipelineState:computePipelineState_];
}


} // /namespace LLGL



// ================================================================================
