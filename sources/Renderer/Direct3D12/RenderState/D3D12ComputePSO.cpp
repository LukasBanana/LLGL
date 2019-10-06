/*
 * D3D12ComputePSO.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12ComputePSO.h"
#include "../D3D12Device.h"
#include "../Shader/D3D12ShaderProgram.h"
#include "../Shader/D3D12Shader.h"
#include "D3D12PipelineLayout.h"
#include "../Command/D3D12CommandContext.h"
#include "../../CheckedCast.h"
#include "../../../Core/Assertion.h"
#include <LLGL/PipelineStateFlags.h>


namespace LLGL
{


D3D12ComputePSO::D3D12ComputePSO(
    D3D12Device&                        device,
    D3D12PipelineLayout&                defaultPipelineLayout,
    const ComputePipelineDescriptor&    desc)
:
    D3D12PipelineState { false, desc.pipelineLayout, defaultPipelineLayout }
{
    /* Validate pointers and get D3D shader program */
    LLGL_ASSERT_PTR(desc.shaderProgram);

    auto shaderProgramD3D = LLGL_CAST(const D3D12ShaderProgram*, desc.shaderProgram);
    if (shaderProgramD3D->GetCS() == nullptr)
        throw std::runtime_error("cannot create compute pipeline without valid compute shader in shader program");

    CreateNativePSO(device, *shaderProgramD3D);
}

void D3D12ComputePSO::Bind(D3D12CommandContext& commandContext)
{
    /* Set root signature and pipeline state */
    commandContext.SetComputeRootSignature(GetRootSignature());
    commandContext.SetPipelineState(GetNative());
}

void D3D12ComputePSO::CreateNativePSO(
    D3D12Device&                device,
    const D3D12ShaderProgram&   shaderProgram)
{
    /* Create graphics pipeline state and graphics command list */
    D3D12_COMPUTE_PIPELINE_STATE_DESC stateDesc = {};
    {
        stateDesc.pRootSignature    = GetRootSignature();
        stateDesc.CS                = shaderProgram.GetCS()->GetByteCode();
    }
    SetNative(device.CreateDXComputePipelineState(stateDesc));
}


} // /namespace LLGL



// ================================================================================
