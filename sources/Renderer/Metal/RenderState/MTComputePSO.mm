/*
 * MTComputePSO.mm
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
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
    {
        GetMutableReport().Errorf("cannot create Metal compute pipeline without compute shader");
        return;
    }

    id<MTLFunction> kernelFunc = computeShader_->GetNative();
    if (!kernelFunc)
    {
        GetMutableReport().Errorf("cannot create Metal compute pipeline without valid compute kernel function");
        return;
    }

    /* Create native compute pipeline state */
    NSError* error = nullptr;
    computePipelineState_ = CreateNativeComputePipelineState(device, kernelFunc, error);
    if (!computePipelineState_)
        MTThrowIfCreateFailed(error, "MTLComputePipelineState");
}

void MTComputePSO::Bind(id<MTLComputeCommandEncoder> computeEncoder)
{
    [computeEncoder setComputePipelineState:computePipelineState_];

    if (auto* pipelineLayout = GetPipelineLayout())
        pipelineLayout->SetStaticKernelSamplers(computeEncoder);
}


/*
 * ======= Private: =======
 */

id<MTLComputePipelineState> MTComputePSO::CreateNativeComputePipelineState(
    id<MTLDevice>   device,
    id<MTLFunction> function,
    NSError*&       error)
{
    if (NeedsConstantsCache())
    {
        /* Create PSO with reflection to generate constants cache */
        MTLAutoreleasedComputePipelineReflection reflection = nil;
        id<MTLComputePipelineState> pso = [device
            newComputePipelineStateWithFunction:    function
            options:                                (MTLPipelineOptionArgumentInfo | MTLPipelineOptionBufferTypeInfo)
            reflection:                             &reflection
            error:                                  &error
        ];
        CreateConstantsCacheForComputePipeline(reflection);
        return pso;
    }
    else
        return [device newComputePipelineStateWithFunction:function error:&error];
}


} // /namespace LLGL



// ================================================================================
