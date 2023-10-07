/*
 * GLComputePSO.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "GLComputePSO.h"
#include <LLGL/PipelineStateFlags.h>


namespace LLGL
{


GLComputePSO::GLComputePSO(const ComputePipelineDescriptor& desc, PipelineCache* pipelineCache) :
    GLPipelineState { /*isGraphicsPSO:*/ false, desc.pipelineLayout, pipelineCache, { desc.computeShader } }
{
}


} // /namespace LLGL



// ================================================================================
