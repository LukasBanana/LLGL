/*
 * GLShaderProgram.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_GL_SHADER_PROGRAM_H__
#define __LLGL_GL_SHADER_PROGRAM_H__


#include <LLGL/ShaderProgram.h>
#include "../OpenGL.h"


namespace LLGL
{


class GLShaderProgram : public ShaderProgram
{

    public:

        GLShaderProgram();
        ~GLShaderProgram();

        void AttachShader( VertexShader&         vertexShader         ) override;
        void AttachShader( FragmentShader&       fragmentShader       ) override;
        void AttachShader( GeometryShader&       geometryShader       ) override;
        void AttachShader( TessControlShader&    tessControlShader    ) override;
        void AttachShader( TessEvaluationShader& tessEvaluationShader ) override;
        void AttachShader( ComputeShader&        computeShader        ) override;

        bool LinkShaders() override;

        std::string QueryInfoLog() override;

        std::vector<ConstantBufferDescriptor> QueryConstantBuffers() const override;

        void BindVertexAttributes(const std::vector<VertexAttribute>& vertexAttribs) override;

        void BindConstantBuffer(const std::string& name, unsigned int bindingIndex) override;

        //! Returns the shader program ID.
        inline GLuint GetID() const
        {
            return id_;
        }

    private:

        template <typename GLShaderType, typename ShaderType>
        void AttachHWShader(const ShaderType& shader);

        GLuint  id_         = 0;
        bool    linkStatus_ = false;

};


} // /namespace LLGL


#endif



// ================================================================================
