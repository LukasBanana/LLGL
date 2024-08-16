/*
 * MTPipelineState.mm
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "MTPipelineState.h"
#include "MTPipelineLayout.h"
#include "../../CheckedCast.h"
#include "../../PipelineStateUtils.h"
#include "../../../Core/CoreUtils.h"


namespace LLGL
{


MTPipelineState::MTPipelineState(bool isGraphicsPSO, const PipelineLayout* pipelineLayout) :
    isGraphicsPSO_ { isGraphicsPSO }
{
    if (pipelineLayout != nullptr)
        pipelineLayout_ = LLGL_CAST(const MTPipelineLayout*, pipelineLayout);
}

const Report* MTPipelineState::GetReport() const
{
    return (report_ ? &report_ : nullptr);
}


/*
 * ======= Protected: =======
 */

void MTPipelineState::ResetReport(std::string&& text, bool hasErrors)
{
    report_.Reset(std::forward<std::string&&>(text), hasErrors);
}

bool MTPipelineState::NeedsConstantsCache() const
{
    return (pipelineLayout_ != nullptr && !pipelineLayout_->GetUniforms().empty());
}

void MTPipelineState::CreateConstantsCacheForRenderPipeline(MTLRenderPipelineReflection* reflection)
{
    MTShaderReflectionArguments args[2] =
    {
        MTShaderReflectionArguments{ StageFlags::VertexStage,   reflection.vertexArguments   },
        MTShaderReflectionArguments{ StageFlags::FragmentStage, reflection.fragmentArguments },
    };
    constantsCacheLayout_ = MakeUnique<MTConstantsCacheLayout>(args, pipelineLayout_->GetUniforms());
}

void MTPipelineState::CreateConstantsCacheForComputePipeline(MTLComputePipelineReflection* reflection)
{
    MTShaderReflectionArguments args[1] =
    {
        MTShaderReflectionArguments{ StageFlags::ComputeStage, reflection.arguments },
    };
    constantsCacheLayout_ = MakeUnique<MTConstantsCacheLayout>(args, pipelineLayout_->GetUniforms());
}


} // /namespace LLGL



// ================================================================================
