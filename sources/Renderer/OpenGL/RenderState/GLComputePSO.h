/*
 * GLComputePSO.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_GL_COMPUTE_PSO_H
#define LLGL_GL_COMPUTE_PSO_H


#include "GLPipelineState.h"


namespace LLGL
{


struct ComputePipelineDescriptor;
class PipelineCache;
class GLShaderProgram;
class GLStateManager;

class GLComputePSO final : public GLPipelineState
{

    public:

        GLComputePSO(const ComputePipelineDescriptor& desc, PipelineCache* pipelineCache = nullptr);

};


} // /namespace LLGL


#endif



// ================================================================================
