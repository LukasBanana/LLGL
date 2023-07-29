/*
 * GLShaderPipeline.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_GL_SHADER_PIPELINE_H
#define LLGL_GL_SHADER_PIPELINE_H


#include "GLShader.h"
#include "GLPipelineSignature.h"
#include <memory>


namespace LLGL
{


class GLShaderPipeline;
class GLShaderBindingLayout;
class GLStateManager;
class Report;

using GLShaderPipelineSPtr = std::shared_ptr<GLShaderPipeline>;

// Base class of GLShaderProgram (for legacy shaders) and GLProgramPipeline (for separable shaders).
class GLShaderPipeline
{

    public:

        virtual ~GLShaderPipeline() = default;

        // Binds this shader pipeline with the specified GL state manager.
        virtual void Bind(GLStateManager& stateMngr) = 0;

        // Binds the resource names to their respective binding slots for this pipeline.
        virtual void BindResourceSlots(const GLShaderBindingLayout& bindingLayout) = 0;

        // Resets the output report with the shader info logs.
        virtual void QueryInfoLogs(Report& report) = 0;

        // Returns the native pipeline ID. Can be either from glCreateProgramPipelines or glCreateProgram.
        inline GLuint GetID() const
        {
            return id_;
        }

    public:

        // Returns a signed integer of the strict-weak-order (SWO) comparison, and 0 on equality.
        static int CompareSWO(const GLShaderPipeline& lhs, const GLShaderPipeline& rhs);
        static int CompareSWO(const GLShaderPipeline& lhs, const GLPipelineSignature& rhs);

    protected:

        GLShaderPipeline() = default;

        // Initializes the shader pipeline with an ID.
        GLShaderPipeline(GLuint id);

        // Builds the pipeline signature for SWO comparison.
        void BuildSignature(std::size_t numShaders, const Shader* const* shaders, GLShader::Permutation permutation);

        // Stores the native shader pipeline ID.
        inline void SetID(GLuint id)
        {
            id_ = id;
        }

        // Returns the pipeline signature.
        inline const GLPipelineSignature& GetSignature() const
        {
            return signature_;
        }

    private:

        GLuint              id_         = 0; // ID from either glCreateProgramPipelines or glCreateProgram.
        GLPipelineSignature signature_;

};


} // /namespace LLGL


#endif



// ================================================================================
