/*
 * GLShader.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLShader.h"
#include "../GLObjectUtils.h"
#include "../Ext/GLExtensions.h"
#include "../../GLCommon/GLExtensionRegistry.h"
#include "../../GLCommon/GLTypes.h"
#include "../../../Core/Helper.h"
#include "../../../Core/Exception.h"
#include <vector>
#include <sstream>
#include <stdexcept>


namespace LLGL
{


GLShader::GLShader(const ShaderDescriptor& desc) :
    Shader { desc.type }
{
    id_ = glCreateShader(GLTypes::Map(desc.type));
    Build(desc);
}

GLShader::~GLShader()
{
    glDeleteShader(id_);
}

void GLShader::SetName(const char* name)
{
    GLSetObjectLabel(GL_SHADER, GetID(), name);
}

bool GLShader::HasErrors() const
{
    GLint status = 0;
    glGetShaderiv(id_, GL_COMPILE_STATUS, &status);
    return (status == GL_FALSE);
}

std::string GLShader::Disassemble(int flags)
{
    return ""; // dummy
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


/*
 * ======= Protected: =======
 */

bool GLShader::MoveStreamOutputFormat(StreamOutputFormat& streamOutputFormat)
{
    if (!streamOutputFormat_.attributes.empty())
    {
        streamOutputFormat.attributes = std::move(streamOutputFormat_.attributes);
        return true;
    }
    return false;
}


/*
 * ======= Private: =======
 */

void GLShader::Build(const ShaderDescriptor& shaderDesc)
{
    if (IsShaderSourceCode(shaderDesc.sourceType))
        CompileSource(shaderDesc);
    else
        LoadBinary(shaderDesc);
}

void GLShader::CompileSource(const ShaderDescriptor& shaderDesc)
{
    /* Get source code */
    std::string fileContent;
    const GLchar* strings[1];

    if (shaderDesc.sourceType == ShaderSourceType::CodeFile)
    {
        fileContent = ReadFileString(shaderDesc.source);
        strings[0]  = fileContent.c_str();
    }
    else
    {
        strings[0] = shaderDesc.source;
    }

    /* Load shader source code, then compile shader */
    glShaderSource(id_, 1, strings, nullptr);
    glCompileShader(id_);

    /* Store stream-output format */
    streamOutputFormat_ = shaderDesc.streamOutput.format;
}

void GLShader::LoadBinary(const ShaderDescriptor& shaderDesc)
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
        glShaderBinary(1, &id_, GL_SHADER_BINARY_FORMAT_SPIR_V, binaryBuffer, binaryLength);

        /* Specialize for the default "main" function in a SPIR-V module  */
        const char* entryPoint = (shaderDesc.entryPoint == nullptr || *shaderDesc.entryPoint == '\0' ? "main" : shaderDesc.entryPoint);
        glSpecializeShader(id_, entryPoint, 0, nullptr, nullptr);

        /* Store stream-output format */
        streamOutputFormat_ = shaderDesc.streamOutput.format;
    }
    else
    #endif
    {
        ThrowNotSupportedExcept(__FUNCTION__, "loading binary shader");
    }
}


} // /namespace LLGL



// ================================================================================
