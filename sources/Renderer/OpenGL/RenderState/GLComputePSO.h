/*
 * GLComputePSO.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_COMPUTE_PSO_H
#define LLGL_GL_COMPUTE_PSO_H


#include "GLPipelineState.h"


namespace LLGL
{


struct ComputePipelineDescriptor;
class GLShaderProgram;
class GLStateManager;

class GLComputePSO final : public GLPipelineState
{

    public:

        GLComputePSO(const ComputePipelineDescriptor& desc);

};


} // /namespace LLGL


#endif



// ================================================================================
