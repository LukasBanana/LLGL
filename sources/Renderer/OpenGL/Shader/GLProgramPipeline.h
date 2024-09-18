/*
 * GLProgramPipeline.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_GL_PROGRAM_PIPELINE_H
#define LLGL_GL_PROGRAM_PIPELINE_H


#include "GLShaderPipeline.h"


namespace LLGL
{


#if GL_ARB_separate_shader_objects

class GLSeparableShader;

class GLProgramPipeline final : public GLShaderPipeline
{

    public:

        GLProgramPipeline(
            std::size_t             numShaders,
            Shader* const*          shaders,
            GLShader::Permutation   permutation = GLShader::PermutationDefault
        );
        ~GLProgramPipeline();

        void Bind(GLStateManager& stateMngr) override;
        void BindResourceSlots(const GLShaderBindingLayout& bindingLayout) override;
        void QueryInfoLogs(Report& report) override;

    private:

        // Binds the specified separable shaders to this program pipeline and their respective pipeline stages.
        void UseProgramStages(std::size_t numShaders, GLSeparableShader* const* shaders, GLShader::Permutation permutation);

    private:

        GLSeparableShader* separableShaders_[LLGL_MAX_NUM_GL_SHADERS_PER_PIPELINE] = {};

};

#else // GL_ARB_separate_shader_objects

class GLProgramPipeline final : public GLShaderPipeline
{

    public:

        GLProgramPipeline(
            std::size_t             numShaders,
            Shader* const*          shaders,
            GLShader::Permutation   permutation = GLShader::PermutationDefault
        );

        void Bind(GLStateManager& stateMngr) override;
        void BindResourceSlots(const GLShaderBindingLayout& bindingLayout) override;
        void QueryInfoLogs(Report& report) override;

};

#endif // /GL_ARB_separate_shader_objects


} // /namespace LLGL


#endif



// ================================================================================
