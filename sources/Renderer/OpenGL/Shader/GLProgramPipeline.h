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


#if LLGL_GLEXT_SEPARATE_SHADER_OBJECTS

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
        void BindResourceSlots(const GLShaderBindingLayout& bindingLayout, const GLShaderBufferInterfaceMap* bufferInterfaceMap = nullptr) override;
        void QueryInfoLogs(Report& report) override;
        void QueryTexBufferNames(std::set<std::string>& outSamplerBufferNames, std::set<std::string>& outImageBufferNames) const override;

    private:

        // Binds the specified separable shaders to this program pipeline and their respective pipeline stages.
        void UseProgramStages(std::size_t numShaders, GLSeparableShader* const* shaders, GLShader::Permutation permutation);

    private:

        GLSeparableShader* separableShaders_[LLGL_MAX_NUM_GL_SHADERS_PER_PIPELINE] = {};

};

#else // LLGL_GLEXT_SEPARATE_SHADER_OBJECTS

class GLProgramPipeline final : public GLShaderPipeline
{

    public:

        GLProgramPipeline(
            std::size_t             numShaders,
            Shader* const*          shaders,
            GLShader::Permutation   permutation = GLShader::PermutationDefault
        );

        void Bind(GLStateManager& stateMngr) override;
        void BindResourceSlots(const GLShaderBindingLayout& bindingLayout, const GLShaderBufferInterfaceMap* bufferInterfaceMap = nullptr) override;
        void QueryInfoLogs(Report& report) override;

};

#endif // /LLGL_GLEXT_SEPARATE_SHADER_OBJECTS


} // /namespace LLGL


#endif



// ================================================================================
