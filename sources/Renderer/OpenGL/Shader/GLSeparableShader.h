/*
 * GLSeparableShader.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_SEPARABLE_SHADER_PROGRAM_H
#define LLGL_GL_SEPARABLE_SHADER_PROGRAM_H


#include "GLShader.h"
#include "../../../Core/BasicReport.h"


namespace LLGL
{


class GLShaderBindingLayout;

// Shader implementation for separable GL shader programs; requires GL_ARB_separate_shader_objects extension.
class GLSeparableShader final : public GLShader
{

    public:

        void SetName(const char* name) override;
        bool Reflect(ShaderReflection& reflection) const override;

    public:

        GLSeparableShader(const ShaderDescriptor& desc);
        ~GLSeparableShader();

        // Binds the resource names to their respective binding slots for this separable shader. Also implemented in GLShaderProgram.
        void BindResourceSlots(const GLShaderBindingLayout& bindingLayout);

        // Queries the program info log and appends it to the output text.
        void QueryInfoLog(std::string& text, bool& hasErrors);

    private:

        const GLShaderBindingLayout* bindingLayout_ = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
