/*
 * GLComputePipeline.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_COMPUTE_PIPELINE_H
#define LLGL_GL_COMPUTE_PIPELINE_H


#include <LLGL/ComputePipeline.h>


namespace LLGL
{


struct ComputePipelineDescriptor;
class GLShaderProgram;
class GLStateManager;

class GLComputePipeline final : public ComputePipeline
{

    public:

        GLComputePipeline(const ComputePipelineDescriptor& desc);

        void Bind(GLStateManager& stateMngr);

        // Returns the shader program used for this graphics pipeline.
        inline const GLShaderProgram* GetShaderProgram() const
        {
            return shaderProgram_;
        }

    private:

        const GLShaderProgram* shaderProgram_ = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
