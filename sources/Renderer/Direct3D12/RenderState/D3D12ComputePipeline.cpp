/*
 * D3D12ComputePipeline.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12ComputePipeline.h"
#include "../D3D12Device.h"
#include "../D3D12ObjectUtils.h"
#include "../Shader/D3D12ShaderProgram.h"
#include "../Shader/D3D12Shader.h"
#include "D3D12PipelineLayout.h"
#include "../../CheckedCast.h"
#include "../../../Core/Assertion.h"
#include <LLGL/ComputePipelineFlags.h>


namespace LLGL
{


D3D12ComputePipeline::D3D12ComputePipeline(
    D3D12Device&                        device,
    ID3D12RootSignature*                defaultRootSignature,
    const ComputePipelineDescriptor&    desc)
{
    /* Validate pointers and get D3D shader program */
    LLGL_ASSERT_PTR(desc.shaderProgram);

    auto shaderProgramD3D = LLGL_CAST(const D3D12ShaderProgram*, desc.shaderProgram);

    if (shaderProgramD3D->GetCS() == nullptr)
        throw std::runtime_error("cannot create compute pipeline without valid compute shader in shader program");

    if (auto pipelineLayout = desc.pipelineLayout)
    {
        /* Create pipeline state with root signature from pipeline layout */
        auto pipelineLayoutD3D = LLGL_CAST(const D3D12PipelineLayout*, pipelineLayout);
        CreatePipelineState(device, *shaderProgramD3D, pipelineLayoutD3D->GetRootSignature());
    }
    else
    {
        /* Create pipeline state with default root signature */
        CreatePipelineState(device, *shaderProgramD3D, defaultRootSignature);
    }
}

void D3D12ComputePipeline::SetName(const char* name)
{
    D3D12SetObjectName(pipelineState_.Get(), name);
}

void D3D12ComputePipeline::Bind(ID3D12GraphicsCommandList* commandList)
{
    /* Set root signature and pipeline state */
    commandList->SetComputeRootSignature(rootSignature_);
    commandList->SetPipelineState(pipelineState_.Get());
}

void D3D12ComputePipeline::CreatePipelineState(
    D3D12Device&                device,
    const D3D12ShaderProgram&   shaderProgram,
    ID3D12RootSignature*        rootSignature)
{
    /* Store used root signature */
    rootSignature_ = rootSignature;

    /* Create graphics pipeline state and graphics command list */
    D3D12_COMPUTE_PIPELINE_STATE_DESC stateDesc = {};
    {
        stateDesc.pRootSignature    = rootSignature;
        stateDesc.CS                = shaderProgram.GetCS()->GetByteCode();
    }
    pipelineState_ = device.CreateDXComputePipelineState(stateDesc);
}


} // /namespace LLGL



// ================================================================================
