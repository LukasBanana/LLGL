/*
 * D3D11ComputePSO.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D11ComputePSO.h"
#include "D3D11StateManager.h"
#include "D3D11PipelineLayout.h"
#include "../Shader/D3D11Shader.h"
#include "../../CheckedCast.h"
#include "../../PipelineStateUtils.h"
#include <LLGL/PipelineStateFlags.h>


namespace LLGL
{


D3D11ComputePSO::D3D11ComputePSO(const ComputePipelineDescriptor& desc) :
    D3D11PipelineState { /*isGraphicsPSO:*/ false, desc.pipelineLayout, GetShadersAsArray(desc) }
{
    /* Convert shader state */
    if (auto computeShaderD3D = LLGL_CAST(const D3D11Shader*, desc.computeShader))
        cs_ = computeShaderD3D->GetNative().cs;
    else
        throw std::invalid_argument("cannot create D3D compute pipeline without compute shader");
}

void D3D11ComputePSO::Bind(D3D11StateManager& stateMngr)
{
    /* Set shader stage */
    stateMngr.SetComputeShader(cs_.Get());

    /* Set static samplers */
    if (const auto* pipelineLayoutD3D = GetPipelineLayout())
        pipelineLayoutD3D->BindComputeStaticSamplers(stateMngr);
}


} // /namespace LLGL



// ================================================================================
