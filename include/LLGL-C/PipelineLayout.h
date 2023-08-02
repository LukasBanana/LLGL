/*
 * PipelineLayout.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_C99_PIPELINE_LAYOUT_H
#define LLGL_C99_PIPELINE_LAYOUT_H


#include <LLGL-C/Export.h>
#include <LLGL-C/Types.h>
#include <stdint.h>


LLGL_C_EXPORT uint32_t llglGetPipelineLayoutNumHeapBindings(LLGLPipelineLayout pipelineLayout);
LLGL_C_EXPORT uint32_t llglGetPipelineLayoutNumBindings(LLGLPipelineLayout pipelineLayout);
LLGL_C_EXPORT uint32_t llglGetPipelineLayoutNumStaticSamplers(LLGLPipelineLayout pipelineLayout);
LLGL_C_EXPORT uint32_t llglGetPipelineLayoutNumUniforms(LLGLPipelineLayout pipelineLayout);


#endif



// ================================================================================
