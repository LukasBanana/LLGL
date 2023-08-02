/*
 * C99PipelineLayout.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/PipelineLayout.h>
#include <LLGL-C/PipelineLayout.h>
#include "C99Internal.h"


// namespace LLGL {


using namespace LLGL;

LLGL_C_EXPORT uint32_t llglGetPipelineLayoutNumHeapBindings(LLGLPipelineLayout pipelineLayout)
{
    return LLGL_PTR(PipelineLayout, pipelineLayout)->GetNumHeapBindings();
}

LLGL_C_EXPORT uint32_t llglGetPipelineLayoutNumBindings(LLGLPipelineLayout pipelineLayout)
{
    return LLGL_PTR(PipelineLayout, pipelineLayout)->GetNumBindings();
}

LLGL_C_EXPORT uint32_t llglGetPipelineLayoutNumStaticSamplers(LLGLPipelineLayout pipelineLayout)
{
    return LLGL_PTR(PipelineLayout, pipelineLayout)->GetNumStaticSamplers();
}

LLGL_C_EXPORT uint32_t llglGetPipelineLayoutNumUniforms(LLGLPipelineLayout pipelineLayout)
{
    return LLGL_PTR(PipelineLayout, pipelineLayout)->GetNumUniforms();
}


// } /namespace LLGL



// ================================================================================
