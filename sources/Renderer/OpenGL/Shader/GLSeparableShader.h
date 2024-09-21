/*
 * GLSeparableShader.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_GL_SEPARABLE_SHADER_PROGRAM_H
#define LLGL_GL_SEPARABLE_SHADER_PROGRAM_H


#include "GLShader.h"
#include <LLGL/Report.h>


namespace LLGL
{


class GLLegacyShader;
class GLShaderBindingLayout;

#if LLGL_GLEXT_SEPARATE_SHADER_OBJECTS

// Shader implementation for separable GL shader programs; requires GL_ARB_separate_shader_objects extension.
class GLSeparableShader final : public GLShader
{

    public:

        void SetDebugName(const char* name) override;
        bool Reflect(ShaderReflection& reflection) const override;

    public:

        GLSeparableShader(const ShaderDescriptor& desc);
        ~GLSeparableShader();

        // Binds the resource names to their respective binding slots for this separable shader. Also implemented in GLShaderProgram.
        void BindResourceSlots(const GLShaderBindingLayout& bindingLayout);

        // Queries the program info log and appends it to the output text.
        void QueryInfoLog(std::string& text, bool& hasErrors);

    private:

        bool CreateAndLinkSeparableGLProgram(GLLegacyShader& intermediateShader, Permutation permutation);

    private:

        const GLShaderBindingLayout* bindingLayout_ = nullptr;

};

#else // LLGL_GLEXT_SEPARATE_SHADER_OBJECTS

class GLSeparableShader final : public GLShader
{

    public:

        void SetDebugName(const char* name) override;
        bool Reflect(ShaderReflection& reflection) const override;

    public:

        GLSeparableShader(const ShaderDescriptor& desc);

        void BindResourceSlots(const GLShaderBindingLayout& bindingLayout);
        void QueryInfoLog(std::string& text, bool& hasErrors);

};

#endif // /LLGL_GLEXT_SEPARATE_SHADER_OBJECTS


} // /namespace LLGL


#endif



// ================================================================================
