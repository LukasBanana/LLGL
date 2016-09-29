/*
 * GLShader.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_GL_SHADER_H__
#define __LLGL_GL_SHADER_H__


#include <LLGL/Shader.h>
#include "../OpenGL.h"


namespace LLGL
{


class GLShader : public Shader
{

    public:

        GLShader(const ShaderType type);
        ~GLShader();

        bool Compile(const ShaderSource& shaderSource) override;

        std::string QueryInfoLog() override;

        //! Returns the hardware shader ID.
        inline GLuint GetID() const
        {
            return id_;
        }

    private:

        GLuint id_ = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
