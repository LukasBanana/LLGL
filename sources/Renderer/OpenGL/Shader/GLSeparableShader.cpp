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
#include "../../../Core/Exception.h"
#include <LLGL/Utils/ForRange.h>


namespace LLGL
{


#if LLGL_GLEXT_SEPARATE_SHADER_OBJECTS

GLSeparableShader::GLSeparableShader(const ShaderDescriptor& desc) :
    GLShader { /*isSeparable:*/ true, desc }
{
    GLLegacyShader intermediateShader{ desc };
    if (CreateAndLinkSeparableGLProgram(intermediateShader, PermutationDefault))
    {
        if (intermediateShader.GetID(PermutationFlippedYPosition) != 0)
            CreateAndLinkSeparableGLProgram(intermediateShader, PermutationFlippedYPosition);
    }

    if (desc.debugName != nullptr)
        SetDebugName(desc.debugName);
}

GLSeparableShader::~GLSeparableShader()
{
    glDeleteProgram(GetID());
}

void GLSeparableShader::SetDebugName(const char* name)
{
    GLSetObjectLabel(GL_PROGRAM, GetID(), name);
}

bool GLSeparableShader::Reflect(ShaderReflection& reflection) const
{
    GLShaderProgram::QueryReflection(GetID(), GetGLType(), reflection);
    return true;
}

void GLSeparableShader::BindResourceSlots(const GLShaderBindingLayout& bindingLayout)
{
    if (bindingLayout_ != &bindingLayout)
    {
        bindingLayout.UniformAndBlockBinding(GetID());
        if (GLuint id = GetID(PermutationFlippedYPosition))
            bindingLayout.UniformAndBlockBinding(id);
        bindingLayout_ = &bindingLayout;
    }
}

void GLSeparableShader::QueryInfoLog(std::string& text, bool& hasErrors)
{
    if (!GLShaderProgram::GetLinkStatus(GetID()))
        hasErrors = true;
    text += GLShaderProgram::GetGLProgramLog(GetID());
}


/*
 * ======= Private: =======
 */

static GLuint CreateSeparableGLProgram()
{
    if (const GLuint program = glCreateProgram())
    {
        glProgramParameteri(program, GL_PROGRAM_SEPARABLE, GL_TRUE);
        return program;
    }
    return 0;
}

bool GLSeparableShader::CreateAndLinkSeparableGLProgram(GLLegacyShader& intermediateShader, Permutation permutation)
{
    /* Create new separable GL program for current permutation */
    const GLuint program = CreateSeparableGLProgram();
    SetID(program, permutation);

    /* Compile and attach intermediate GL shader object */
    const GLuint shader = intermediateShader.GetID(permutation);
    glAttachShader(program, shader);

    switch (intermediateShader.GetType())
    {
        case ShaderType::Vertex:
            /* Build input layout for vertex shader */
            GLShaderProgram::BindAttribLocations(program, GetNumVertexAttribs(), GetVertexAttribs());
            break;

        case ShaderType::Fragment:
            /* Build output layout for fragment shader */
            GLShaderProgram::BindFragDataLocations(program, GetNumFragmentAttribs(), GetFragmentAttribs());
            break;

        default:
            break;
    }

    /* Build transform feedback varyings for vertex or geometry shader and link program */
    const auto& varyings = GetTransformFeedbackVaryings();
    if (!varyings.empty())
        GLShaderProgram::LinkProgramWithTransformFeedbackVaryings(program, varyings.size(), varyings.data());
    else
        GLShaderProgram::LinkProgram(program);

    /* Detach intermediate shader before it gets deleted */
    glDetachShader(program, shader);

    /* Query link status and log */
    const bool status = GLShaderProgram::GetLinkStatus(program);
    ReportStatusAndLog(status, GLShaderProgram::GetGLProgramLog(program));

    return status;
}

#else // LLGL_GLEXT_SEPARATE_SHADER_OBJECTS

GLSeparableShader::GLSeparableShader(const ShaderDescriptor& desc) :
    GLShader { /*isSeparable:*/ true, desc }
{
    LLGL_TRAP_FEATURE_NOT_SUPPORTED("GL_ARB_separate_shader_objects");
}

void GLSeparableShader::SetDebugName(const char* name)
{
    // dummy
}

bool GLSeparableShader::Reflect(ShaderReflection& reflection) const
{
    return false; // dummy
}

void GLSeparableShader::BindResourceSlots(const GLShaderBindingLayout& bindingLayout)
{
    // dummy
}

void GLSeparableShader::QueryInfoLog(std::string& text, bool& hasErrors)
{
    // dummy
}

#endif // /LLGL_GLEXT_SEPARATE_SHADER_OBJECTS


} // /namespace LLGL



// ================================================================================
