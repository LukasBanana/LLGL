/*
 * DbgPipelineState.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "DbgPipelineState.h"
#include "DbgPipelineLayout.h"
#include "../../PipelineStateUtils.h"
#include "../DbgCore.h"
#include "../../CheckedCast.h"


namespace LLGL
{


DbgPipelineState::DbgPipelineState(PipelineState& instance, const GraphicsPipelineDescriptor& desc) :
    instance       { instance                                                 },
    pipelineLayout { LLGL_CAST(const DbgPipelineLayout*, desc.pipelineLayout) },
    isGraphicsPSO  { true                                                     },
    graphicsDesc   { desc                                                     }
{
}

DbgPipelineState::DbgPipelineState(PipelineState& instance, const ComputePipelineDescriptor& desc) :
    instance       { instance                                                 },
    pipelineLayout { LLGL_CAST(const DbgPipelineLayout*, desc.pipelineLayout) },
    isGraphicsPSO  { false                                                    },
    computeDesc    { desc                                                     }
{
}

DbgPipelineState::~DbgPipelineState()
{
    // dummy
}

bool DbgPipelineState::HasDynamicBlendFactor() const
{
    return (isGraphicsPSO && graphicsDesc.blend.blendFactorDynamic && IsBlendFactorEnabled(graphicsDesc.blend));
}

bool DbgPipelineState::HasDynamicStencilRef() const
{
    return (isGraphicsPSO && graphicsDesc.stencil.referenceDynamic && IsStencilRefEnabled(graphicsDesc.stencil));
}

void DbgPipelineState::SetName(const char* name)
{
    DbgSetObjectName(*this, name);
}

const Report* DbgPipelineState::GetReport() const
{
    return instance.GetReport();
}


} // /namespace LLGL



// ================================================================================
