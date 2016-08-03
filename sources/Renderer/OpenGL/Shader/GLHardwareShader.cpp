/*
 * GLHardwareShader.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLHardwareShader.h"
#include "../GLExtensions.h"
#include <vector>


namespace LLGL
{


GLHardwareShader::GLHardwareShader(GLenum shaderType)
{
    id_ = glCreateShader(shaderType);
}

GLHardwareShader::~GLHardwareShader()
{
    glDeleteShader(id_);
}

bool GLHardwareShader::Compile(const std::string& shaderSource)
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

std::string GLHardwareShader::QueryInfoLog()
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
