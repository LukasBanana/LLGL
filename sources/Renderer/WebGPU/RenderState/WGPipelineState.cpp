/*
 * WGPipelineState.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "WGPipelineState.h"
//#include "../../../Core/Assertion.h"


namespace LLGL
{


WGPipelineState::WGPipelineState(bool isRenderPipeline) :
    isRenderPipeline_ { isRenderPipeline }
{
}

const Report* WGPipelineState::GetReport() const
{
    return (report_ ? &report_ : nullptr);
}


} // /namespace LLGL



// ================================================================================
