/*
 * GLShader.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLShader.h"
#include "../Ext/GLExtensions.h"
#include "../../GLCommon/GLExtensionRegistry.h"
#include "../../GLCommon/GLTypes.h"
#include <vector>
#include <sstream>
#include <stdexcept>


namespace LLGL
{


GLShader::GLShader(const ShaderType type) :
    Shader { type }
{
    id_ = glCreateShader(GLTypes::Map(type));
}

GLShader::~GLShader()
{
    glDeleteShader(id_);
}

bool GLShader::Compile(const std::string& sourceCode, const ShaderDescriptor& shaderDesc)
{
    /* Load shader source code, then compile shader */
    const GLchar* strings[] = { sourceCode.c_str() };

    glShaderSource(id_, 1, strings, nullptr);
    glCompileShader(id_);

    /* Store stream-output format */
    streamOutputFormat_ = shaderDesc.streamOutput.format;

    /* Query compilation status */
    return QueryCompileStatus();
}

bool GLShader::LoadBinary(std::vector<char>&& binaryCode, const ShaderDescriptor& shaderDesc)
{
    #if defined GL_ARB_gl_spirv && defined GL_ARB_ES2_compatibility
    if (HasExtension(GLExt::ARB_gl_spirv) && HasExtension(GLExt::ARB_ES2_compatibility))
    {
        /* Load shader binary */
        glShaderBinary(1, &id_, GL_SHADER_BINARY_FORMAT_SPIR_V, binaryCode.data(), static_cast<GLsizei>(binaryCode.size()));

        /* Specialize for the default "main" function in a SPIR-V module  */
        const char* entryPoint = (shaderDesc.entryPoint.empty() ? "main" : shaderDesc.entryPoint.c_str());
        glSpecializeShader(id_, entryPoint, 0, nullptr, nullptr);

        /* Store stream-output format */
        streamOutputFormat_ = shaderDesc.streamOutput.format;

        /* Query compilation status */
        return QueryCompileStatus();
    }
    #endif
    return false;
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

bool GLShader::QueryCompileStatus() const
{
    GLint status = 0;
    glGetShaderiv(id_, GL_COMPILE_STATUS, &status);
    return (status != GL_FALSE);
}


} // /namespace LLGL



// ================================================================================
