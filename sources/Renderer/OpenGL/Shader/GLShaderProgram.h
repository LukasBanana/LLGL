/*
 * GLShaderProgram.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_SHADER_PROGRAM_H
#define LLGL_GL_SHADER_PROGRAM_H


#include <LLGL/ShaderProgram.h>
#include "GLShaderUniform.h"
#include "../../GLCommon/OpenGL.h"


namespace LLGL
{


class GLShaderProgram : public ShaderProgram
{

    public:

        GLShaderProgram();
        ~GLShaderProgram();

        void AttachShader(Shader& shader) override;
        void DetachAll() override;

        bool LinkShaders() override;

        std::string QueryInfoLog() override;

        std::vector<VertexAttribute> QueryVertexAttributes() const override;
        std::vector<StreamOutputAttribute> QueryStreamOutputAttributes() const override;
        std::vector<ConstantBufferViewDescriptor> QueryConstantBuffers() const override;
        std::vector<StorageBufferViewDescriptor> QueryStorageBuffers() const override;
        std::vector<UniformDescriptor> QueryUniforms() const override;

        void BuildInputLayout(const VertexFormat& vertexFormat) override;
        void BindConstantBuffer(const std::string& name, unsigned int bindingIndex) override;
        void BindStorageBuffer(const std::string& name, unsigned int bindingIndex) override;

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

        bool QueryActiveAttribs(
            GLenum attribCountType, GLenum attribNameLengthType,
            GLint& numAttribs, GLint& maxNameLength, std::vector<char>& nameBuffer
        ) const;

        bool LinkShaderProgram();

        void BuildTransformFeedbackVaryingsEXT(const std::vector<StreamOutputAttribute>& attributes);
    
        #ifndef __APPLE__
        void BuildTransformFeedbackVaryingsNV(const std::vector<StreamOutputAttribute>& attributes);
        #endif

        GLuint              id_ = 0;

        GLShaderUniform     uniform_;

        bool                hasFragmentShader_ = false;

        StreamOutputFormat  streamOutputFormat_;

};


} // /namespace LLGL


#endif



// ================================================================================
