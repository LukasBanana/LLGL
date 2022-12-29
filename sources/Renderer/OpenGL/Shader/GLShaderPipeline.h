/*
 * GLShaderPipeline.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_SHADER_PIPELINE_H
#define LLGL_GL_SHADER_PIPELINE_H


#include "GLPipelineSignature.h"
#include <memory>


namespace LLGL
{


class GLShaderPipeline;
class GLShaderBindingLayout;
class GLStateManager;
class BasicReport;

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

        // Adds the shader info logs to the output report.
        virtual void QueryInfoLogs(BasicReport& report) = 0;

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
        void BuildSignature(std::size_t numShaders, const Shader* const* shaders);

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
