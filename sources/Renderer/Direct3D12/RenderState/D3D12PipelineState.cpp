/*
 * D3D12PipelineState.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12PipelineState.h"
#include "D3D12PipelineLayout.h"
#include "../D3D12Device.h"
#include "../D3D12ObjectUtils.h"
#include "../../CheckedCast.h"


namespace LLGL
{


D3D12PipelineState::D3D12PipelineState(
    bool                    isGraphicsPSO,
    ID3D12RootSignature*    defaultRootSignature,
    const PipelineLayout*   pipelineLayout)
:
    isGraphicsPSO_ { isGraphicsPSO }
{
    if (pipelineLayout != nullptr)
    {
        /* Create pipeline state with root signature from pipeline layout */
        auto pipelineLayoutD3D = LLGL_CAST(const D3D12PipelineLayout*, pipelineLayout);
        rootSignature_ = pipelineLayoutD3D->GetRootSignature();
    }
    else
    {
        /* Create pipeline state with default root signature */
        rootSignature_ = defaultRootSignature;
    }
}

void D3D12PipelineState::SetName(const char* name)
{
    D3D12SetObjectName(native_.Get(), name);
}

void D3D12PipelineState::SetNative(ComPtr<ID3D12PipelineState>&& native)
{
    native_ = std::move(native);
}


} // /namespace LLGL



// ================================================================================
