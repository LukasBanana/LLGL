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
#include "../D3D12Serialization.h"
#include "../../CheckedCast.h"
#include "../../DXCommon/DXCore.h"


namespace LLGL
{


D3D12PipelineState::D3D12PipelineState(
    bool                    isGraphicsPSO,
    const PipelineLayout*   pipelineLayout,
    D3D12PipelineLayout&    defaultPipelineLayout)
:
    isGraphicsPSO_ { isGraphicsPSO }
{
    if (pipelineLayout != nullptr)
    {
        /* Create pipeline state with root signature from pipeline layout */
        auto pipelineLayoutD3D = LLGL_CAST(const D3D12PipelineLayout*, pipelineLayout);
        rootSignature_ = pipelineLayoutD3D->GetSharedRootSignature();
    }
    else
    {
        /* Create pipeline state with default root signature */
        rootSignature_ = defaultPipelineLayout.GetSharedRootSignature();
    }
}

D3D12PipelineState::D3D12PipelineState(
    bool                            isGraphicsPSO,
    ID3D12Device*                   device,
    Serialization::Deserializer&    reader)
:
    isGraphicsPSO_ { isGraphicsPSO }
{
    /* Create root signature from cache */
    auto seg = reader.ReadSegment(Serialization::D3D12Ident_RootSignature);
    auto hr = device->CreateRootSignature(0, seg.data, seg.size, IID_PPV_ARGS(rootSignature_.ReleaseAndGetAddressOf()));
    DXThrowIfFailed(hr, "failed to create D3D12 root signature");
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
