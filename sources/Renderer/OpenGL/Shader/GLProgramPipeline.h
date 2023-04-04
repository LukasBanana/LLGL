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


class GLSeparableShader;

class GLProgramPipeline final : public GLShaderPipeline
{

    public:

        GLProgramPipeline(std::size_t numShaders, Shader* const* shaders);
        ~GLProgramPipeline();

        void Bind(GLStateManager& stateMngr) override;
        void BindResourceSlots(const GLShaderBindingLayout& bindingLayout) override;
        void QueryInfoLogs(BasicReport& report) override;

    private:

        // Binds the specified separable shaders to this program pipeline and their respective pipeline stages.
        void UseProgramStages(std::size_t numShaders, GLSeparableShader* const* shaders);

    private:

        GLSeparableShader* separableShaders_[LLGL_MAX_NUM_GL_SHADERS_PER_PIPELINE] = {};

};


} // /namespace LLGL


#endif



// ================================================================================
