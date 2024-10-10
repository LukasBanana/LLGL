/*
 * D3D12BuiltinShaderFactory.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D12BuiltinShaderFactory.h"
#include "Builtin/D3D12Builtin.h"
#include "../Shader/D3D12RootSignature.h"
#include "../../DXCommon/DXCore.h"
#include "../../DXCommon/DXTypes.h"
#include "../../../Core/Exception.h"


namespace LLGL
{


D3D12BuiltinShaderFactory& D3D12BuiltinShaderFactory::Get()
{
    static D3D12BuiltinShaderFactory instance;
    return instance;
}

void D3D12BuiltinShaderFactory::CreateBuiltinPSOs(ID3D12Device* device)
{
    D3D12RootSignature rootSignature;
    {
        rootSignature.ResetAndAlloc(2, 0);
        rootSignature[0].InitAsConstants(0, 1);
        rootSignature[1].InitAsDescriptor(D3D12_ROOT_PARAMETER_TYPE_UAV, 0);
    }
    CreateComputePSO(device, D3D12BuiltinPSO::StreamOutputDrawArgsCS, rootSignature, LLGL_IDR_STREAMOUTPUTDRAWARGS_CS, sizeof(LLGL_IDR_STREAMOUTPUTDRAWARGS_CS));
}

void D3D12BuiltinShaderFactory::Clear()
{
    for (ComPtr<ID3D12RootSignature>& nativeRootSignature : rootSignatures_)
        nativeRootSignature.Reset();
    for (ComPtr<ID3D12PipelineState>& nativePipelineState : builtinPSOs_)
        nativePipelineState.Reset();
}

bool D3D12BuiltinShaderFactory::GetBulitinPSO(const D3D12BuiltinPSO builtin, ID3D12PipelineState*& outPipelineState, ID3D12RootSignature*& outRootSignature) const
{
    const std::size_t idx = static_cast<std::size_t>(builtin);
    if (idx < D3D12BuiltinShaderFactory::numBuiltinShaders)
    {
        outPipelineState = builtinPSOs_[idx].Get();
        outRootSignature = rootSignatures_[idx].Get();
        return true;
    }
    return false;
}

void D3D12BuiltinShaderFactory::CreateComputePSO(
    ID3D12Device*           device,
    D3D12BuiltinPSO         builtin,
    D3D12RootSignature&     rootSignature,
    const unsigned char*    shaderBytecode,
    size_t                  shaderBytecodeSize)
{
    std::size_t psoIndex = static_cast<std::size_t>(builtin);

    rootSignatures_[psoIndex] = rootSignature.Finalize(device);

    ComPtr<ID3D12PipelineState> pipelineState;

    if (ComPtr<ID3DBlob> blob = DXCreateBlob(shaderBytecode, shaderBytecodeSize))
    {
        /* Create graphics pipeline state and graphics command list */
        D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
        {
            psoDesc.pRootSignature      = rootSignatures_[psoIndex].Get();
            psoDesc.CS.pShaderBytecode  = blob->GetBufferPointer();
            psoDesc.CS.BytecodeLength   = blob->GetBufferSize();
        }
        HRESULT hr = device->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(pipelineState.ReleaseAndGetAddressOf()));
        DXThrowIfCreateFailed(hr, "ID3D12PipelineState");
    }

    builtinPSOs_[psoIndex] = pipelineState;
}


} // /namespace LLGL



// ================================================================================
