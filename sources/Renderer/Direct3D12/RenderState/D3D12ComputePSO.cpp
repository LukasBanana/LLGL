/*
 * D3D12ComputePSO.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D12ComputePSO.h"
#include "D3D12PipelineCache.h"
#include "D3D12PipelineLayout.h"
#include "../D3D12Device.h"
#include "../Shader/D3D12Shader.h"
#include "../Command/D3D12CommandContext.h"
#include "../../DXCommon/DXCore.h"
#include "../../CheckedCast.h"
#include "../../PipelineStateUtils.h"
#include <LLGL/PipelineStateFlags.h>


namespace LLGL
{


D3D12ComputePSO::D3D12ComputePSO(
    ID3D12Device*                       device,
    D3D12PipelineLayout&                defaultPipelineLayout,
    const ComputePipelineDescriptor&    desc,
    PipelineCache*                      pipelineCache)
:
    D3D12PipelineState { /*isGraphicsPSO:*/ false, desc.pipelineLayout, GetShadersAsArray(desc), defaultPipelineLayout }
{
    auto* computeShaderD3D = LLGL_CAST(const D3D12Shader*, desc.computeShader);
    if (computeShaderD3D == nullptr)
    {
        ResetReport("cannot create D3D compute PSO without compute shader", true);
        return;
    }

    /* Create native compute PSO */
    if (pipelineCache != nullptr)
    {
        auto* pipelineCacheD3D = LLGL_CAST(D3D12PipelineCache*, pipelineCache);
        CreateNativePSO(device, computeShaderD3D->GetByteCode(), desc.debugName, pipelineCacheD3D);
    }
    else
        CreateNativePSO(device, computeShaderD3D->GetByteCode(), desc.debugName);

    if (desc.debugName != nullptr)
        SetDebugName(desc.debugName);
}

void D3D12ComputePSO::Bind(D3D12CommandContext& commandContext)
{
    /* Set root signature and pipeline state */
    commandContext.SetComputeRootSignature(GetRootSignature());
    commandContext.SetPipelineState(GetNative());
}

void D3D12ComputePSO::CreateNativePSO(
    ID3D12Device*                   device,
    const D3D12_SHADER_BYTECODE&    csBytecode,
    const char*                     debugName,
    D3D12PipelineCache*             pipelineCache)
{
    /* Create graphics pipeline state and graphics command list */
    D3D12_COMPUTE_PIPELINE_STATE_DESC stateDesc = {};

    stateDesc.pRootSignature    = GetRootSignature();
    stateDesc.CS                = csBytecode;

    /* Set PSO cache if specified */
    if (pipelineCache != nullptr)
        stateDesc.CachedPSO = pipelineCache->GetCachedPSO();

    /* Create native PSO */
    SetNativeAndUpdateCache(CreateNativePSOWithDesc(device, stateDesc, debugName), pipelineCache);
}

ComPtr<ID3D12PipelineState> D3D12ComputePSO::CreateNativePSOWithDesc(ID3D12Device* device, const D3D12_COMPUTE_PIPELINE_STATE_DESC& desc, const char* debugName)
{
    ComPtr<ID3D12PipelineState> pipelineState;
    HRESULT hr = device->CreateComputePipelineState(&desc, IID_PPV_ARGS(pipelineState.ReleaseAndGetAddressOf()));
    if (FAILED(hr))
    {
        GetMutableReport().Errorf("Failed to create D3D12 compute pipelines state [%s] (HRESULT = %s)\n", debugName, DXErrorToStrOrHex(hr));
        return nullptr;
    }
    return pipelineState;
}


} // /namespace LLGL



// ================================================================================
