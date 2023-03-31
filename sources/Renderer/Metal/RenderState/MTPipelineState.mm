/*
 * MTPipelineState.mm
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "MTPipelineState.h"
#include "MTPipelineLayout.h"
#include "../../CheckedCast.h"
#include "../../../Core/CoreUtils.h"


namespace LLGL
{


MTPipelineState::MTPipelineState(bool isGraphicsPSO, const PipelineLayout* pipelineLayout) :
    isGraphicsPSO_ { isGraphicsPSO }
{
    if (pipelineLayout != nullptr)
    {
        pipelineLayout_ = LLGL_CAST(const MTPipelineLayout*, pipelineLayout);
        if (!pipelineLayout_->GetDynamicBindings().empty())
            descriptorCache_ = MakeUnique<MTDescriptorCache>(pipelineLayout_->GetDynamicBindings());
    }
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
