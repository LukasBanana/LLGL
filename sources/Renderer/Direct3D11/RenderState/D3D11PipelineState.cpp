/*
 * D3D11PipelineState.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11PipelineState.h"


namespace LLGL
{


const Report* D3D11PipelineState::GetReport() const
{
    return (*report_.GetText() != '\0' || report_.HasErrors() ? &report_ : nullptr);
}

void D3D11PipelineState::ResetReport(std::string&& text, bool hasErrors)
{
    report_.Reset(std::forward<std::string&&>(text), hasErrors);
}


} // /namespace LLGL



// ================================================================================
