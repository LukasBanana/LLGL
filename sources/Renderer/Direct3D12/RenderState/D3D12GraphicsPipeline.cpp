/*
 * D3D12GraphicsPipeline.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12GraphicsPipeline.h"
#include "../DXCore.h"
#include "../../CheckedCast.h"
#include "../../../Core/Helper.h""


namespace LLGL
{


D3D12GraphicsPipeline::D3D12GraphicsPipeline(ID3D12Device* device, ID3D12CommandAllocator* commandAlloc, const GraphicsPipelineDescriptor& desc)
{
    HRESULT hr = 0;

    /* Setup D3D12 graphics pipeline descriptor */
    D3D12_GRAPHICS_PIPELINE_STATE_DESC stateDesc;
    InitMemory(stateDesc);

    stateDesc.NumRenderTargets = 8;
    for (UINT i = 0; i < stateDesc.NumRenderTargets; ++i)
        stateDesc.RTVFormats[i] = DXGI_FORMAT_B8G8R8A8_UNORM;

    /* Create D3D12 graphics pipeline sate */
    hr = device->CreateGraphicsPipelineState(&stateDesc, IID_PPV_ARGS(&pipelineState_));
    DXThrowIfFailed(hr, "failed to create D3D12 graphics pipeline state");

    /* Create command list */
    hr = device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAlloc, pipelineState_, IID_PPV_ARGS(&commandList_));
    DXThrowIfFailed(hr, "failed to create D3D12 command list");
}

D3D12GraphicsPipeline::~D3D12GraphicsPipeline()
{
    SafeRelease(pipelineState_);
    SafeRelease(commandList_);
}

/*void D3D12GraphicsPipeline::Bind(D3D12StateManager& stateMngr)
{
}*/


} // /namespace LLGL



// ================================================================================
