/*
 * GLShader.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_SHADER_H
#define LLGL_GL_SHADER_H


#include <LLGL/Shader.h>
#include "../OpenGL.h"


namespace LLGL
{


class GLShader : public Shader
{

    public:

        GLShader(const ShaderDescriptor& desc);
        ~GLShader();

        bool HasErrors() const override;

        std::string Disassemble(int flags = 0) override;

        std::string QueryInfoLog() override;

        // Returns the native shader ID.
        inline GLuint GetID() const
        {
            return id_;
        }

    protected:

        friend class GLShaderProgram;

        bool MoveStreamOutputFormat(StreamOutputFormat& streamOutputFormat);

    private:

        void Build(const ShaderDescriptor& shaderDesc);
        void CompileSource(const ShaderDescriptor& shaderDesc);
        void LoadBinary(const ShaderDescriptor& shaderDesc);

        GLuint              id_ = 0;
        StreamOutputFormat  streamOutputFormat_;

};


} // /namespace LLGL


#endif



// ================================================================================
