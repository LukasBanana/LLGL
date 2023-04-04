/*
 * D3D12ComputePSO.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D12ComputePSO.h"
#include "D3D12PipelineLayout.h"
#include "../D3D12Device.h"
#include "../Shader/D3D12Shader.h"
#include "../Command/D3D12CommandContext.h"
#include "../../CheckedCast.h"
#include "../../PipelineStateUtils.h"
#include "../../../Core/Assertion.h"
#include <LLGL/PipelineStateFlags.h>


namespace LLGL
{


D3D12ComputePSO::D3D12ComputePSO(
    D3D12Device&                        device,
    D3D12PipelineLayout&                defaultPipelineLayout,
    const ComputePipelineDescriptor&    desc)
:
    D3D12PipelineState { /*isGraphicsPSO:*/ false, desc.pipelineLayout, GetShadersAsArray(desc), defaultPipelineLayout }
{
    if (auto computeShaderD3D = LLGL_CAST(const D3D12Shader*, desc.computeShader))
        CreateNativePSO(device, computeShaderD3D->GetByteCode());
    else
        throw std::runtime_error("cannot create D3D compute pipeline without compute shader");
}

void D3D12ComputePSO::Bind(D3D12CommandContext& commandContext)
{
    /* Set root signature and pipeline state */
    commandContext.SetComputeRootSignature(GetRootSignature());
    commandContext.SetPipelineState(GetNative());
}

void D3D12ComputePSO::CreateNativePSO(D3D12Device& device, const D3D12_SHADER_BYTECODE& csBytecode)
{
    /* Create graphics pipeline state and graphics command list */
    D3D12_COMPUTE_PIPELINE_STATE_DESC stateDesc = {};
    {
        stateDesc.pRootSignature    = GetRootSignature();
        stateDesc.CS                = csBytecode;
    }
    SetNative(device.CreateDXComputePipelineState(stateDesc));
}


} // /namespace LLGL



// ================================================================================
