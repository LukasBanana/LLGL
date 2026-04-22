/*
 * WGPipelineState.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "WGPipelineState.h"
#include "WGPipelineLayout.h"
#include "../../CheckedCast.h"


namespace LLGL
{


WGPipelineState::WGPipelineState(bool isRenderPipeline) :
    isRenderPipeline_ { isRenderPipeline }
{
}

WGPUPipelineLayout WGPipelineState::CreatePipelineLayoutPermutation(
    const PipelineLayout*                       pipelineLayout,
    WGPUDevice                                  device,
    ArrayView<const WGResourceReflectionTable*> resourceTables)
{
    if (pipelineLayout != nullptr)
    {
        const WGPipelineLayout* pipelineLayoutWG = LLGL_CAST(const WGPipelineLayout*, pipelineLayout);
        pipelineLayoutPermutation_ = pipelineLayoutWG->CreatePermutation(device, resourceTables, GetMutableReport());
        return pipelineLayoutPermutation_->GetNative();
    }
    return nullptr;
}

const Report* WGPipelineState::GetReport() const
{
    return (report_ ? &report_ : nullptr);
}


} // /namespace LLGL



// ================================================================================
