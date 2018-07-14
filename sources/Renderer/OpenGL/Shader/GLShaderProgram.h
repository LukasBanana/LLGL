/*
 * GLShaderProgram.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_SHADER_PROGRAM_H
#define LLGL_GL_SHADER_PROGRAM_H


#include <LLGL/ShaderProgram.h>
#include "GLShaderUniform.h"
#include "../OpenGL.h"


namespace LLGL
{


class GLShaderProgram final : public ShaderProgram
{

    public:

        GLShaderProgram(const ShaderProgramDescriptor& desc);
        ~GLShaderProgram();

        bool HasErrors() const override;

        std::string QueryInfoLog() override;

        ShaderReflectionDescriptor QueryReflectionDesc() const override;

        void BindConstantBuffer(const std::string& name, std::uint32_t bindingIndex) override;
        void BindStorageBuffer(const std::string& name, std::uint32_t bindingIndex) override;

        ShaderUniform* LockShaderUniform() override;
        void UnlockShaderUniform() override;

        // Returns the shader program ID.
        inline GLuint GetID() const
        {
            return id_;
        }

        // Returns true if this shader program has a fragment shader.
        inline bool HasFragmentShader() const
        {
            return hasFragmentShader_;
        }

    private:

        void Attach(Shader* shader);
        void BuildInputLayout(std::size_t numVertexFormats, const VertexFormat* vertexFormats);
        void Link();

        bool QueryActiveAttribs(
            GLenum attribCountType, GLenum attribNameLengthType,
            GLint& numAttribs, GLint& maxNameLength, std::vector<char>& nameBuffer
        ) const;

        void BuildTransformFeedbackVaryingsEXT(const std::vector<StreamOutputAttribute>& attributes);
        #ifndef __APPLE__
        void BuildTransformFeedbackVaryingsNV(const std::vector<StreamOutputAttribute>& attributes);
        #endif

        void Reflect(ShaderReflectionDescriptor& reflection) const;
        void QueryVertexAttributes(ShaderReflectionDescriptor& reflection) const;
        void QueryStreamOutputAttributes(ShaderReflectionDescriptor& reflection) const;
        void QueryConstantBuffers(ShaderReflectionDescriptor& reflection) const;
        void QueryStorageBuffers(ShaderReflectionDescriptor& reflection) const;
        void QueryUniforms(ShaderReflectionDescriptor& reflection) const;

        #ifdef GL_ARB_program_interface_query
        void QueryBufferProperties(ShaderReflectionDescriptor::ResourceView& resourceView, GLenum programInterface, GLuint resourceIndex) const;
        #endif // /GL_ARB_program_interface_query

        GLuint              id_ = 0;

        GLShaderUniform     uniform_;

        bool                hasFragmentShader_  = false;
        bool                isLinked_           = false;

        StreamOutputFormat  streamOutputFormat_;

};


} // /namespace LLGL


#endif



// ================================================================================
