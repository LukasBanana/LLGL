/*
 * GLComputePSO.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLComputePSO.h"
#include <LLGL/PipelineStateFlags.h>


namespace LLGL
{


GLComputePSO::GLComputePSO(const ComputePipelineDescriptor& desc) :
    GLPipelineState { false, desc.pipelineLayout, desc.shaderProgram }
{
}


} // /namespace LLGL



// ================================================================================
