/*
 * GLShader.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLShader.h"
#include "../Ext/GLExtensions.h"
#include "../GLTypes.h"
#include <vector>
#include <sstream>
#include <stdexcept>


namespace LLGL
{


GLShader::GLShader(const ShaderType type) :
    Shader( type )
{
    id_ = glCreateShader(GLTypes::Map(type));
}

GLShader::~GLShader()
{
    glDeleteShader(id_);
}

bool GLShader::Compile(const std::string& shaderSource)
{
    /* Setup shader source */
    const GLchar* strings[] = { shaderSource.c_str() };
    glShaderSource(id_, 1, strings, nullptr);

    /* Compile shader */
    glCompileShader(id_);

    /* Query compilation status */
    GLint compileStatus = 0;
    glGetShaderiv(id_, GL_COMPILE_STATUS, &compileStatus);

    return (compileStatus != GL_FALSE);
}

bool GLShader::Compile(const std::string&, const std::string&, const std::string&, int)
{
    throw std::runtime_error("invalid 'Shader::Compile' function for GLSL");
}

std::string GLShader::QueryInfoLog()
{
    /* Query info log length */
    GLint infoLogLength = 0;
    glGetShaderiv(id_, GL_INFO_LOG_LENGTH, &infoLogLength);

    if (infoLogLength > 0)
    {
        /* Store info log in byte buffer (because GL writes it's own null-terminator character!) */
        std::vector<char> infoLog;
        infoLog.resize(infoLogLength, '\0');

        /* Query info log output */
        GLsizei charsWritten = 0;
        glGetShaderInfoLog(id_, infoLogLength, &charsWritten, infoLog.data());

        /* Convert byte buffer to string */
        return std::string(infoLog.data());
    }

    return "";
}


} // /namespace LLGL



// ================================================================================
