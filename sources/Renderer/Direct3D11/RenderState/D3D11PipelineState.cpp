/*
 * D3D11PipelineState.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D11PipelineState.h"
#include "D3D11PipelineLayout.h"
#include "D3D11ConstantsCache.h"
#include "../Shader/D3D11Shader.h"
#include "../../CheckedCast.h"
#include "../../PipelineStateUtils.h"
#include "../../../Core/CoreUtils.h"
#include "../../../Core/ReportUtils.h"


namespace LLGL
{


static bool IsAnyShaderInvalid(const ArrayView<D3D11Shader*>& shaders)
{
    for (D3D11Shader* shader : shaders)
    {
        if (shader->GetByteCode() == nullptr)
            return true;
    }
    return false;
}

D3D11PipelineState::D3D11PipelineState(
    bool                        isGraphicsPSO,
    const PipelineLayout*       pipelineLayout,
    const ArrayView<Shader*>&   shaders)
:
    isGraphicsPSO_  { isGraphicsPSO                                         },
    pipelineLayout_ { LLGL_CAST(const D3D11PipelineLayout*, pipelineLayout) }
{
    if (pipelineLayout_ != nullptr && !pipelineLayout_->GetUniforms().empty())
    {
        SmallVector<D3D11Shader*, 5> shadersD3D = CastShaderArray<D3D11Shader>(shaders);
        if (!IsAnyShaderInvalid(shadersD3D))
            constantsCache_ = MakeUnique<D3D11ConstantsCache>(shadersD3D, pipelineLayout_->GetUniforms());
    }
}

const Report* D3D11PipelineState::GetReport() const
{
    return (*report_.GetText() != '\0' || report_.HasErrors() ? &report_ : nullptr);
}

void D3D11PipelineState::ResetReport(std::string&& text, bool hasErrors)
{
    ResetReportWithNewline(report_, std::forward<std::string&&>(text), hasErrors);
}


} // /namespace LLGL



// ================================================================================
