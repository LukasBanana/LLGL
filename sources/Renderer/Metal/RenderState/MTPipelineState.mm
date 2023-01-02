/*
 * MTPipelineState.mm
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "MTPipelineState.h"


namespace LLGL
{


MTPipelineState::MTPipelineState(bool isGraphicsPSO/*, const ShaderProgram* shaderProgram*/) :
    isGraphicsPSO_ { isGraphicsPSO }
{
}

const Report* MTPipelineState::GetReport() const
{
    return (report_ ? &report_ : nullptr);
}

void MTPipelineState::ResetReport(std::string&& text, bool hasErrors)
{
    report_.Reset(std::forward<std::string&&>(text), hasErrors);
}


} // /namespace LLGL



// ================================================================================
