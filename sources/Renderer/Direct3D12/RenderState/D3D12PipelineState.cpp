/*
 * D3D12PipelineState.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D12PipelineState.h"
#include "D3D12PipelineLayout.h"
#include "../D3D12Device.h"
#include "../D3D12ObjectUtils.h"
#include "../D3D12Serialization.h"
#include "../Shader/D3D12Shader.h"
#include "../../CheckedCast.h"
#include "../../DXCommon/DXCore.h"
#include "../../PipelineStateUtils.h"


namespace LLGL
{


D3D12PipelineState::D3D12PipelineState(
    bool                        isGraphicsPSO,
    const PipelineLayout*       pipelineLayout,
    const ArrayView<Shader*>&   shaders,
    D3D12PipelineLayout&        defaultPipelineLayout)
:
    isGraphicsPSO_ { isGraphicsPSO }
{
    if (pipelineLayout != nullptr)
    {
        /* Create pipeline state with root signature from pipeline layout */
        pipelineLayout_ = LLGL_CAST(const D3D12PipelineLayout*, pipelineLayout);

        if (pipelineLayout_->NeedsRootConstantPermutation())
            rootSignature_ = pipelineLayout_->CreateRootSignatureWith32BitConstants(CastShaderArray<D3D12Shader>(shaders), rootConstantMap_);
        else
            rootSignature_ = pipelineLayout_->GetFinalizedRootSignature();
    }
    else
    {
        /* Create pipeline state with default root signature */
        rootSignature_ = defaultPipelineLayout.GetFinalizedRootSignature();
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

const Report* D3D12PipelineState::GetReport() const
{
    return (*report_.GetText() != '\0' || report_.HasErrors() ? &report_ : nullptr);
}

void D3D12PipelineState::SetNative(ComPtr<ID3D12PipelineState>&& native)
{
    native_ = std::move(native);
}

void D3D12PipelineState::ResetReport(std::string&& text, bool hasErrors)
{
    report_.Reset(std::forward<std::string&&>(text), hasErrors);
}


} // /namespace LLGL



// ================================================================================
