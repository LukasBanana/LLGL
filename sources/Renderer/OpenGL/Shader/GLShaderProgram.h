/*
 * GLShaderProgram.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_GL_SHADER_PROGRAM_H
#define LLGL_GL_SHADER_PROGRAM_H


#include <LLGL/ShaderReflection.h>
#include <LLGL/VertexAttribute.h>
#include <LLGL/Container/ArrayView.h>
#include "GLShaderPipeline.h"
#include "GLShaderUniform.h"
#include <string>
#include <set>


namespace LLGL
{


struct GLShaderAttribute;
class GLShaderBindingLayout;
class GLPipelineCache;

class GLShaderProgram final : public GLShaderPipeline
{

    public:

        void Bind(GLStateManager& stateMngr) override;
        void BindResourceSlots(const GLShaderBindingLayout& bindingLayout, const GLShaderBufferInterfaceMap* bufferInterfaceMap = nullptr) override;
        void QueryInfoLogs(Report& report) override;
        void QueryTexBufferNames(std::set<std::string>& outSamplerBufferNames, std::set<std::string>& outImageBufferNames) const override;

    public:

        GLShaderProgram(
            ArrayView<const Shader*>    shaders,
            ArrayView<VertexAttribute>  inputVertexAttribs  = {},
            ArrayView<VertexAttribute>  outputVertexAttribs = {},
            GLShader::Permutation       permutation         = GLShader::PermutationDefault,
            GLPipelineCache*            pipelineCache       = nullptr
        );

        // Wrapper for mutable shaders array view.
        GLShaderProgram(
            ArrayView<Shader*>          shaders,
            ArrayView<VertexAttribute>  inputVertexAttribs  = {},
            ArrayView<VertexAttribute>  outputVertexAttribs = {},
            GLShader::Permutation       permutation         = GLShader::PermutationDefault,
            GLPipelineCache*            pipelineCache       = nullptr
        );

        ~GLShaderProgram();

    public:

        // Returns true if the native GL shader program was linked successfully.
        static bool GetLinkStatus(GLuint program);

        // Returns the native GL shader program log.
        static std::string GetGLProgramLog(GLuint program);

        // Invokes glBindAttribLocation on the specified program for all vertex attributes.
        static void BindAttribLocations(GLuint program, ArrayView<GLShaderAttribute> vertexAttribs, bool isCaseInsensitive);

        // Invokes glBindFragDataLocation on the specified program for all fragment attributes.
        static void BindFragDataLocations(GLuint program, ArrayView<GLShaderAttribute> fragmentAttribs);

        // Links the specified GL program and binds the transform feedback varyings.
        static void LinkProgramWithTransformFeedbackVaryings(GLuint program, ArrayView<const char*> varyings);

        // Simply links the GL program.
        static void LinkProgram(GLuint program);

        // Queries the shader reflection for the specified program.
        static void QueryReflection(GLuint program, GLenum shaderStage, ShaderReflection& reflection);

        // Queries all texture buffer names of the specified program and inserts them into the set.
        static void QueryTexBufferNames(GLuint program, std::set<std::string>& samplerBufferNames, std::set<std::string>& imageBufferNames);

    private:

        // Main function for constructor to attach shaders, build attributes, and link program.
        void BuildProgramBinary(
            ArrayView<const Shader*>    shaders,
            ArrayView<VertexAttribute>  inputVertexAttribs,
            ArrayView<VertexAttribute>  outputVertexAttribs,
            GLShader::Permutation       permutation
        );

    private:

        const GLShaderBindingLayout*    bindingLayout_          = nullptr;

        #if LLGL_USE_NULL_FRAGMENT_SHADER
        bool                            hasNullFragmentShader_  = false;
        #endif

};


} // /namespace LLGL


#endif



// ================================================================================
