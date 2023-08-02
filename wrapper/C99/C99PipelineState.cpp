/*
 * C99PipelineState.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/PipelineState.h>
#include <LLGL-C/PipelineState.h>
#include "C99Internal.h"


// namespace LLGL {


using namespace LLGL;

LLGL_C_EXPORT LLGLReport llglGetPipelineStateReport(LLGLPipelineState pipelineState)
{
    return LLGLReport{ LLGL_PTR(PipelineState, pipelineState)->GetReport() };
}


// } /namespace LLGL



// ================================================================================
