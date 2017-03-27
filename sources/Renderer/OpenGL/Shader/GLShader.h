/*
 * GLShader.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
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

        GLShader(const ShaderType type);
        ~GLShader();

        bool Compile(const std::string& sourceCode, const ShaderDescriptor& shaderDesc = {}) override;

        bool LoadBinary(std::vector<char>&& binaryCode, const ShaderDescriptor& shaderDesc = {}) override;

        std::string Disassemble(int flags = 0) override;

        std::string QueryInfoLog() override;

        //! Returns the hardware shader ID.
        inline GLuint GetID() const
        {
            return id_;
        }

    protected:

        friend class GLShaderProgram;

        bool MoveStreamOutputFormat(StreamOutputFormat& streamOutputFormat);

    private:

        GLuint              id_ = 0;

        StreamOutputFormat  streamOutputFormat_;

};


} // /namespace LLGL


#endif



// ================================================================================
