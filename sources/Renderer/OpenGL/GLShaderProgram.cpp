/*
 * GLShaderProgram.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLShaderProgram.h"
#include "GLVertexShader.h"
#include "GLFragmentShader.h"
#include "GLGeometryShader.h"
#include "GLTessControlShader.h"
#include "GLTessEvaluationShader.h"
#include "GLComputeShader.h"
#include "GLExtensions.h"
#include "../CheckedCast.h"
#include <vector>


namespace LLGL
{


GLShaderProgram::GLShaderProgram()
{
    id_ = glCreateProgram();
}

GLShaderProgram::~GLShaderProgram()
{
    glDeleteProgram(id_);
}

void GLShaderProgram::AttachShader(VertexShader& vertexShader)
{
    AttachHWShader<GLVertexShader>(vertexShader);
}

void GLShaderProgram::AttachShader(FragmentShader& fragmentShader)
{
    AttachHWShader<GLFragmentShader>(fragmentShader);
}

void GLShaderProgram::AttachShader(GeometryShader& geometryShader)
{
    AttachHWShader<GLGeometryShader>(geometryShader);
}

void GLShaderProgram::AttachShader(TessControlShader& tessControlShader)
{
    AttachHWShader<GLTessControlShader>(tessControlShader);
}

void GLShaderProgram::AttachShader(TessEvaluationShader& tessEvaluationShader)
{
    AttachHWShader<GLTessEvaluationShader>(tessEvaluationShader);
}

void GLShaderProgram::AttachShader(ComputeShader& computeShader)
{
    AttachHWShader<GLComputeShader>(computeShader);
}

bool GLShaderProgram::LinkShaders()
{
    /* Link shader program */
    glLinkProgram(id_);

    /* Query linking status */
    GLint linkStatus = 0;
    glGetProgramiv(id_, GL_LINK_STATUS, &linkStatus);

    return (linkStatus != GL_FALSE);
}

std::string GLShaderProgram::QueryInfoLog()
{
    /* Query info log length */
    GLint infoLogLength = 0;
    glGetProgramiv(id_, GL_INFO_LOG_LENGTH, &infoLogLength);

    if (infoLogLength > 0)
    {
        /* Store info log in byte buffer (because GL writes it's own null-terminator character!) */
        std::vector<char> infoLog;
        infoLog.resize(infoLogLength, '\0');

        /* Query info log output */
        GLsizei charsWritten = 0;
        glGetProgramInfoLog(id_, infoLogLength, &charsWritten, infoLog.data());

        /* Convert byte buffer to string */
        return std::string(infoLog.data());
    }

    return "";
}


/*
 * ======= Private: ========
 */

template <typename GLShaderType, typename ShaderType>
void GLShaderProgram::AttachHWShader(const ShaderType& shader)
{
    const auto& shaderGL = LLGL_CAST(const GLShaderType&, shader);
    glAttachShader(id_, shaderGL.hwShader.GetID());
}


} // /namespace LLGL



// ================================================================================
