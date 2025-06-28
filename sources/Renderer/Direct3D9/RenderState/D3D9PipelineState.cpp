/*
 * D3D9PipelineState.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D9PipelineState.h"
#include "../D3D9Types.h"


namespace LLGL
{


D3D9PipelineState::D3D9PipelineState(const GraphicsPipelineDescriptor& desc, bool isProgrammablePipeline) :
    isProgrammablePipeline_ { isProgrammablePipeline }
{
}

void D3D9PipelineState::SetDebugName(const char* name)
{
    // dummy
}


} // /namespace LLGL



// ================================================================================
