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
    BuildShader(desc);
    if (desc.debugName != nullptr)
        SetDebugName(desc.debugName);
}

GLLegacyShader::~GLLegacyShader()
{
    glDeleteShader(GetID());
}

void GLLegacyShader::SetDebugName(const char* name)
{
    GLSetObjectLabel(GL_SHADER, GetID(), name);
}

bool GLLegacyShader::Reflect(ShaderReflection& reflection) const
{
    const Shader* shaders[] = { this };
    GLShaderProgram intermediateProgram{ 1, shaders };
    GLShaderProgram::QueryReflection(intermediateProgram.GetID(), GetGLType(), reflection);
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

GLuint GLLegacyShader::CreateShaderPermutation(Permutation permutation)
{
    GLuint id = glCreateShader(GLTypes::Map(GetType()));
    SetID(id, permutation);
    return id;
}

bool GLLegacyShader::FinalizeShaderPermutation(Permutation permutation)
{
    /* Query compile status and log */
    const bool status = GLLegacyShader::GetCompileStatus(GetID());
    ReportStatusAndLog(status, GLLegacyShader::GetGLShaderLog(GetID()));
    return status;
}

void GLLegacyShader::BuildShader(const ShaderDescriptor& shaderDesc)
{
    if (IsShaderSourceCode(shaderDesc.sourceType))
        CompileSource(shaderDesc);
    else
        LoadBinary(shaderDesc);
}

void GLLegacyShader::CompileSource(const ShaderDescriptor& shaderDesc)
{
    auto CompileShaderPermutation = [this, &shaderDesc](Permutation permutation, long enabledFlags) -> bool
    {
        const GLuint shader = CreateShaderPermutation(permutation);
        auto sourceCallback = std::bind(GLLegacyShader::CompileShaderSource, shader, std::placeholders::_1);

        if (shaderDesc.sourceType == ShaderSourceType::CodeFile)
        {
            const std::string fileContent = ReadFileString(shaderDesc.source);
            GLShader::PatchShaderSource(sourceCallback, fileContent.c_str(), shaderDesc, enabledFlags);
        }
        else
            GLShader::PatchShaderSource(sourceCallback, shaderDesc.source, shaderDesc, enabledFlags);

        return FinalizeShaderPermutation(permutation);
    };

    /* Compile and patch default shader permutation */
    if (CompileShaderPermutation(PermutationDefault, ShaderCompileFlags::NoOptimization))
    {
        /* Compile and patch shader permutation for flipped Y-position */
        if (GLShader::NeedsPermutationFlippedYPosition(shaderDesc.type, shaderDesc.flags))
            CompileShaderPermutation(PermutationFlippedYPosition, ShaderCompileFlags::NoOptimization | ShaderCompileFlags::PatchClippingOrigin);
    }
}

void GLLegacyShader::LoadBinary(const ShaderDescriptor& shaderDesc)
{
    #if LLGL_GLEXT_GL_SPIRV
    const GLuint shader = CreateShaderPermutation(PermutationDefault);

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
        glShaderBinary(1, &shader, GL_SHADER_BINARY_FORMAT_SPIR_V, binaryBuffer, binaryLength);

        /* Specialize for the default "main" function in a SPIR-V module  */
        const char* entryPoint = (shaderDesc.entryPoint == nullptr || *shaderDesc.entryPoint == '\0' ? "main" : shaderDesc.entryPoint);
        glSpecializeShader(shader, entryPoint, 0, nullptr, nullptr);
    }
    else
    #endif // /LLGL_GLEXT_GL_SPIRV
    {
        LLGL_TRAP_FEATURE_NOT_SUPPORTED("loading binary shader");
    }

    FinalizeShaderPermutation(PermutationDefault);
}


} // /namespace LLGL



// ================================================================================
