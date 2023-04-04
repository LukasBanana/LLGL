/*
 * GLLegacyShader.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "GLLegacyShader.h"
#include "GLShaderProgram.h"
#include "../Ext/GLExtensions.h"
#include "../Ext/GLExtensionRegistry.h"
#include "../GLTypes.h"
#include "../GLObjectUtils.h"
#include "../../../Core/Exception.h"


namespace LLGL
{


GLLegacyShader::GLLegacyShader(const ShaderDescriptor& desc) :
    GLShader { /*isSeparable:*/ false, desc }
{
    GLuint id = glCreateShader(GLTypes::Map(desc.type));
    SetID(id);
    BuildShader(desc);

    /* Query compile status and log */
    ReportStatusAndLog(
        GLLegacyShader::GetCompileStatus(GetID()),
        GLLegacyShader::GetGLShaderLog(GetID())
    );
}

GLLegacyShader::~GLLegacyShader()
{
    glDeleteShader(GetID());
}

void GLLegacyShader::SetName(const char* name)
{
    GLSetObjectLabel(GL_SHADER, GetID(), name);
}

bool GLLegacyShader::Reflect(ShaderReflection& reflection) const
{
    const Shader* shaders[] = { this };
    GLShaderProgram intermediateProgram{ 1, shaders };
    GLShaderProgram::QueryReflection(intermediateProgram.GetID(), reflection);
    return true;
}

void GLLegacyShader::CompileShaderSource(GLuint shader, const char* source)
{
    const GLchar* strings[1] = { source };
    glShaderSource(shader, 1, strings, nullptr);
    glCompileShader(shader);
}

bool GLLegacyShader::GetCompileStatus(GLuint shader)
{
    GLint status = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    return (status != GL_FALSE);
}

std::string GLLegacyShader::GetGLShaderLog(GLuint shader)
{
    /* Query info log length */
    GLint infoLogLength = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);

    if (infoLogLength > 0)
    {
        /* Store info log in byte buffer (because GL writes it's own null-terminator character!) */
        std::vector<char> infoLog;
        infoLog.resize(infoLogLength, '\0');

        /* Query info log output */
        GLsizei charsWritten = 0;
        glGetShaderInfoLog(shader, infoLogLength, &charsWritten, infoLog.data());

        /* Convert byte buffer to string */
        return std::string(infoLog.data());
    }

    return "";
}


/*
 * ======= Private: =======
 */

void GLLegacyShader::BuildShader(const ShaderDescriptor& shaderDesc)
{
    if (IsShaderSourceCode(shaderDesc.sourceType))
        CompileSource(shaderDesc);
    else
        LoadBinary(shaderDesc);
}

void GLLegacyShader::CompileSource(const ShaderDescriptor& shaderDesc)
{
    GLShader::PatchShaderSource(
        [this](const char* source)
        {
            GLLegacyShader::CompileShaderSource(GetID(), source);
        },
        shaderDesc
    );
}

void GLLegacyShader::LoadBinary(const ShaderDescriptor& shaderDesc)
{
    #if defined GL_ARB_gl_spirv && defined GL_ARB_ES2_compatibility
    if (HasExtension(GLExt::ARB_gl_spirv) && HasExtension(GLExt::ARB_ES2_compatibility))
    {
        /* Get shader binary */
        std::vector<char>   fileContent;
        const void*         binaryBuffer    = nullptr;
        GLsizei             binaryLength    = 0;

        if (shaderDesc.sourceType == ShaderSourceType::BinaryFile)
        {
            /* Load binary from file */
            fileContent = ReadFileBuffer(shaderDesc.source);
            binaryBuffer = fileContent.data();
            binaryLength = static_cast<GLsizei>(fileContent.size());
        }
        else
        {
            /* Load binary from buffer */
            binaryBuffer = shaderDesc.source;
            binaryLength = static_cast<GLsizei>(shaderDesc.sourceSize);
        }

        /* Load shader binary */
        const GLuint shaders[] = { GetID() };
        glShaderBinary(1, shaders, GL_SHADER_BINARY_FORMAT_SPIR_V, binaryBuffer, binaryLength);

        /* Specialize for the default "main" function in a SPIR-V module  */
        const char* entryPoint = (shaderDesc.entryPoint == nullptr || *shaderDesc.entryPoint == '\0' ? "main" : shaderDesc.entryPoint);
        glSpecializeShader(GetID(), entryPoint, 0, nullptr, nullptr);
    }
    else
    #endif
    {
        ThrowNotSupportedExcept(__FUNCTION__, "loading binary shader");
    }
}


} // /namespace LLGL



// ================================================================================
