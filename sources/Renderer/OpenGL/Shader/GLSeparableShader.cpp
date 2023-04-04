/*
 * GLSeparableShader.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "GLSeparableShader.h"
#include "GLLegacyShader.h"
#include "GLShaderProgram.h"
#include "GLShaderBindingLayout.h"
#include "../Ext/GLExtensions.h"
#include "../GLTypes.h"
#include "../GLObjectUtils.h"


namespace LLGL
{


static GLuint CreateSeparableGLProgram()
{
    if (const GLuint program = glCreateProgram())
    {
        glProgramParameteri(program, GL_PROGRAM_SEPARABLE, GL_TRUE);
        return program;
    }
    return 0;
}

GLSeparableShader::GLSeparableShader(const ShaderDescriptor& desc) :
    GLShader { /*isSeparable:*/ true, desc }
{
    const GLuint program = CreateSeparableGLProgram();
    SetID(program);

    /* Compile and attach intermediate GL shader object */
    GLLegacyShader intermediateShader{ desc };
    glAttachShader(GetID(), intermediateShader.GetID());

    switch (desc.type)
    {
        case ShaderType::Vertex:
            /* Build input layout for vertex shader */
            GLShaderProgram::BindAttribLocations(GetID(), GetNumVertexAttribs(), GetVertexAttribs());
            break;

        case ShaderType::Fragment:
            /* Build output layout for fragment shader */
            GLShaderProgram::BindFragDataLocations(GetID(), GetNumFragmentAttribs(), GetFragmentAttribs());
            break;

        default:
            break;
    }

    /* Build transform feedback varyings for vertex or geometry shader and link program */
    const auto& varyings = GetTransformFeedbackVaryings();
    if (!varyings.empty())
        GLShaderProgram::LinkProgramWithTransformFeedbackVaryings(GetID(), varyings.size(), varyings.data());
    else
        GLShaderProgram::LinkProgram(GetID());

    /* Detach intermediate shader before it gets deleted */
    glDetachShader(GetID(), intermediateShader.GetID());

    /* Query link status and log */
    ReportStatusAndLog(
        GLShaderProgram::GetLinkStatus(GetID()),
        GLShaderProgram::GetGLProgramLog(GetID())
    );
}

GLSeparableShader::~GLSeparableShader()
{
    glDeleteProgram(GetID());
}

void GLSeparableShader::SetName(const char* name)
{
    GLSetObjectLabel(GL_PROGRAM, GetID(), name);
}

bool GLSeparableShader::Reflect(ShaderReflection& reflection) const
{
    GLShaderProgram::QueryReflection(GetID(), reflection);
    return true;
}

void GLSeparableShader::BindResourceSlots(const GLShaderBindingLayout& bindingLayout)
{
    if (bindingLayout_ != &bindingLayout)
    {
        bindingLayout.UniformAndBlockBinding(GetID());
        bindingLayout_ = &bindingLayout;
    }
}

void GLSeparableShader::QueryInfoLog(std::string& text, bool& hasErrors)
{
    if (!GLShaderProgram::GetLinkStatus(GetID()))
        hasErrors = true;
    text += GLShaderProgram::GetGLProgramLog(GetID());
}


} // /namespace LLGL



// ================================================================================
