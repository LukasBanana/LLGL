/*
 * GLComputePipeline.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_GL_COMPUTE_PIPELINE_H__
#define __LLGL_GL_COMPUTE_PIPELINE_H__


#include <LLGL/ComputePipeline.h>


namespace LLGL
{


class GLShaderProgram;
class GLStateManager;

class GLComputePipeline : public ComputePipeline
{

    public:

        GLComputePipeline(const ComputePipelineDescriptor& desc);

        void Bind(GLStateManager& stateMngr);

    private:

        GLShaderProgram* shaderProgram_ = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
