/*
 * MTComputePSO.mm
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "MTComputePSO.h"
#include "MTPipelineLayout.h"
#include "../MTCore.h"
#include "../Shader/MTShader.h"
#include "../../CheckedCast.h"
#include <LLGL/PipelineStateFlags.h>
#include <string>
#include <stdexcept>


namespace LLGL
{


MTComputePSO::MTComputePSO(id<MTLDevice> device, const ComputePipelineDescriptor& desc) :
    MTPipelineState { /*isGraphicsPSO:*/ false, desc.pipelineLayout }
{
    /* Get native shader functions */
    computeShader_  = LLGL_CAST(const MTShader*, desc.computeShader);
    if (!computeShader_)
        throw std::invalid_argument("cannot create Metal compute pipeline without compute shader");

    id<MTLFunction> kernelFunc = computeShader_->GetNative();
    if (!kernelFunc)
        throw std::invalid_argument("cannot create Metal compute pipeline without valid compute kernel function");

    /* Create native compute pipeline state */
    NSError* error = nullptr;
    computePipelineState_ = [device newComputePipelineStateWithFunction:kernelFunc error:&error];
    if (!computePipelineState_)
        MTThrowIfCreateFailed(error, "MTLComputePipelineState");
}

void MTComputePSO::Bind(id<MTLComputeCommandEncoder> computeEncoder)
{
    [computeEncoder setComputePipelineState:computePipelineState_];

    if (auto* pipelineLayout = GetPipelineLayout())
        pipelineLayout->SetStaticKernelSamplers(computeEncoder);
}


} // /namespace LLGL



// ================================================================================
