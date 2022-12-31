/*
 * NullPipelineState.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "NullPipelineState.h"

namespace LLGL
{


NullPipelineState::NullPipelineState(const GraphicsPipelineDescriptor& desc) :
    isGraphicsPSO { true },
    graphicsDesc  { desc }
{
}

NullPipelineState::NullPipelineState(const ComputePipelineDescriptor& desc) :
    isGraphicsPSO { false },
    computeDesc   { desc  }
{
}

NullPipelineState::~NullPipelineState()
{
    // dummy
}

void NullPipelineState::SetName(const char* name)
{
    if (name != nullptr)
        label_ = name;
    else
        label_.clear();
}

const Report* NullPipelineState::GetReport() const
{
    return nullptr; //TODO
}


} // /namespace LLGL



// ================================================================================
