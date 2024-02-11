/*
 * D3D12PipelineState.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D12PipelineState.h"
#include "D3D12PipelineCache.h"
#include "D3D12PipelineLayout.h"
#include "../D3D12Device.h"
#include "../D3D12ObjectUtils.h"
#include "../Shader/D3D12Shader.h"
#include "../../CheckedCast.h"
#include "../../DXCommon/DXCore.h"
#include "../../PipelineStateUtils.h"
#include "../../../Core/ReportUtils.h"


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

void D3D12PipelineState::SetDebugName(const char* name)
{
    D3D12SetObjectName(native_.Get(), name);
}

const Report* D3D12PipelineState::GetReport() const
{
    return (*report_.GetText() != '\0' || report_.HasErrors() ? &report_ : nullptr);
}

void D3D12PipelineState::SetNativeAndUpdateCache(ComPtr<ID3D12PipelineState>&& native, D3D12PipelineCache* pipelineCache)
{
    /* Store native pipeline state */
    native_ = std::move(native);

    /* Get cached PSO if specified but not initialized */
    if (pipelineCache != nullptr && !pipelineCache->HasInitialBlob())
    {
        ComPtr<ID3DBlob> cachedBlob;
        HRESULT hr = native_->GetCachedBlob(cachedBlob.GetAddressOf());
        DXThrowIfFailed(hr, "failed to retrieve cached blob from ID3D12PipelineState");
        pipelineCache->SetNativeBlob(std::move(cachedBlob));
    }
}

void D3D12PipelineState::ResetReport(std::string&& text, bool hasErrors)
{
    ResetReportWithNewline(report_, std::forward<std::string&&>(text), hasErrors);
}


} // /namespace LLGL



// ================================================================================
