/*
 * GLShader.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLShader.h"
#include "../GLExtensions.h"
#include <vector>
#include <sstream>
#include <stdexcept>


namespace LLGL
{


static GLenum GetGLShaderType(const ShaderType type)
{
    switch (type)
    {
        case ShaderType::Vertex:
            return GL_VERTEX_SHADER;
        case ShaderType::Geometry:
            return GL_GEOMETRY_SHADER;
        case ShaderType::TessControl:
            return GL_TESS_CONTROL_SHADER;
        case ShaderType::TessEvaluation:
            return GL_TESS_EVALUATION_SHADER;
        case ShaderType::Fragment:
            return GL_FRAGMENT_SHADER;
        case ShaderType::Compute:
            return GL_COMPUTE_SHADER;
    }

    /* Invalid type -> throw error */
    std::stringstream err;
    err << "invalid shader type (0x" << std::hex << static_cast<int>(type) << ")";
    throw std::invalid_argument(err.str());
}

GLShader::GLShader(const ShaderType type) :
    Shader( type )
{
    id_ = glCreateShader(GetGLShaderType(type));
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

bool GLShader::Compile(const std::string& shaderSource, const std::string& entryPoint, const std::string& target)
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
