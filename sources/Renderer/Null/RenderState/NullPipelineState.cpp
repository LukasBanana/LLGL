/*
 * NullPipelineState.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "NullPipelineState.h"

namespace LLGL
{


NullPipelineState::NullPipelineState(const GraphicsPipelineDescriptor& desc) :
    isGraphicsPSO { true },
    graphicsDesc  { desc }
{
    if (desc.debugName != nullptr)
        SetDebugName(desc.debugName);
}

NullPipelineState::NullPipelineState(const ComputePipelineDescriptor& desc) :
    isGraphicsPSO { false },
    computeDesc   { desc  }
{
    if (desc.debugName != nullptr)
        SetDebugName(desc.debugName);
}

void NullPipelineState::SetDebugName(const char* name)
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
