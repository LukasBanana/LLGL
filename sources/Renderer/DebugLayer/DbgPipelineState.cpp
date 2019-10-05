/*
 * DbgPipelineState.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "DbgPipelineState.h"
#include "DbgCore.h"


namespace LLGL
{


DbgPipelineState::DbgPipelineState(PipelineState& instance, const GraphicsPipelineDescriptor& desc) :
    instance      { instance },
    isGraphicsPSO { true     },
    graphicsDesc  { desc     }
{
}

DbgPipelineState::DbgPipelineState(PipelineState& instance, const ComputePipelineDescriptor& desc) :
    instance      { instance },
    isGraphicsPSO { false    },
    computeDesc   { desc     }
{
}

DbgPipelineState::~DbgPipelineState()
{
    // dummy
}

void DbgPipelineState::SetName(const char* name)
{
    DbgSetObjectName(*this, name);
}


} // /namespace LLGL



// ================================================================================
