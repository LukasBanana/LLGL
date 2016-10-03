/*
 * GLShaderProgram.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_GL_SHADER_PROGRAM_H__
#define __LLGL_GL_SHADER_PROGRAM_H__


#include <LLGL/ShaderProgram.h>
#include "GLShaderUniform.h"
#include "../OpenGL.h"


namespace LLGL
{


class GLShaderProgram : public ShaderProgram
{

    public:

        GLShaderProgram();
        ~GLShaderProgram();

        void AttachShader(Shader& shader) override;

        bool LinkShaders() override;

        std::string QueryInfoLog() override;

        std::vector<VertexAttribute> QueryVertexAttributes() const override;
        std::vector<ConstantBufferViewDescriptor> QueryConstantBuffers() const override;
        std::vector<StorageBufferViewDescriptor> QueryStorageBuffers() const override;
        std::vector<UniformDescriptor> QueryUniforms() const override;

        void BuildInputLayout(const std::vector<VertexAttribute>& vertexAttribs) override;
        void BindConstantBuffer(const std::string& name, unsigned int bindingIndex) override;
        void BindStorageBuffer(const std::string& name, unsigned int bindingIndex) override;

        ShaderUniform* LockShaderUniform() override;
        void UnlockShaderUniform() override;

        //! Returns the shader program ID.
        inline GLuint GetID() const
        {
            return id_;
        }

    private:

        GLuint          id_ = 0;

        GLShaderUniform uniform_;

};


} // /namespace LLGL


#endif



// ================================================================================
