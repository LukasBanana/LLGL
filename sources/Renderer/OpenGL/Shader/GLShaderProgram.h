/*
 * GLShaderProgram.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_SHADER_PROGRAM_H
#define LLGL_GL_SHADER_PROGRAM_H


#include <LLGL/ShaderReflection.h>
#include "GLShaderPipeline.h"
#include "GLShaderUniform.h"


namespace LLGL
{


struct GLShaderAttribute;
class GLShaderBindingLayout;

class GLShaderProgram final : public GLShaderPipeline
{

    public:

        void Bind(GLStateManager& stateMngr) override;
        void BindResourceSlots(const GLShaderBindingLayout& bindingLayout) override;
        void QueryInfoLogs(BasicReport& report) override;

    public:

        GLShaderProgram(std::size_t numShaders, Shader* const* shaders);
        ~GLShaderProgram();

        UniformLocation FindUniformLocation(const char* name) const;
        void QueryReflection(ShaderReflection& reflection) const;

    public:

        // Returns true if the native GL shader program was linked successfully.
        static bool GetLinkStatus(GLuint program);

        // Returns the native GL shader program log.
        static std::string GetGLProgramLog(GLuint program);

        // Invokes glBindAttribLocation on the specified program for all vertex attributes.
        static void BindAttribLocations(GLuint program, std::size_t numVertexAttribs, const GLShaderAttribute* vertexAttribs);

        // Invokes glBindFragDataLocation on the specified program for all fragment attributes.
        static void BindFragDataLocations(GLuint program, std::size_t numFragmentAttribs, const GLShaderAttribute* fragmentAttribs);

        // Links the specified GL program and binds the transform feedback varyings.
        static void LinkProgramWithTransformFeedbackVaryings(GLuint program, std::size_t numVaryings, const char* const* varyings);

        // Simply links the GL program.
        static void LinkProgram(GLuint program);

    private:

        bool QueryActiveAttribs(
            GLenum              attribCountType,
            GLenum              attribNameLengthType,
            GLint&              numAttribs,
            GLint&              maxNameLength,
            std::vector<char>&  nameBuffer
        ) const;

        void QueryVertexAttributes(ShaderReflection& reflection) const;
        void QueryStreamOutputAttributes(ShaderReflection& reflection) const;
        void QueryConstantBuffers(ShaderReflection& reflection) const;
        void QueryStorageBuffers(ShaderReflection& reflection) const;
        void QueryUniforms(ShaderReflection& reflection) const;
        void QueryWorkGroupSize(ShaderReflection& reflection) const;

        #ifdef GL_ARB_program_interface_query
        void QueryBufferProperties(ShaderResource& resource, GLenum programInterface, GLuint resourceIndex) const;
        #endif

    private:

        const GLShaderBindingLayout*    bindingLayout_          = nullptr;

        #ifdef __APPLE__
        bool                            hasNullFragmentShader_  = false;
        #endif

};


} // /namespace LLGL


#endif



// ================================================================================
