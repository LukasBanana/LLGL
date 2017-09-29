/*
 * DbgShader.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "DbgShader.h"
#include "DbgCore.h"


namespace LLGL
{


DbgShader::DbgShader(Shader& instance, const ShaderType type, RenderingDebugger* debugger) :
    Shader    { type     },
    instance  { instance },
    debugger_ { debugger }
{
}

bool DbgShader::Compile(const std::string& sourceCode, const ShaderDescriptor& shaderDesc)
{
    compiled_ = instance.Compile(sourceCode, shaderDesc);
    return compiled_;
}

bool DbgShader::LoadBinary(std::vector<char>&& binaryCode, const ShaderDescriptor& shaderDesc)
{
    compiled_ = instance.LoadBinary(std::move(binaryCode), shaderDesc);
    return compiled_;
}

std::string DbgShader::Disassemble(int flags)
{
    if (debugger_)
    {
        LLGL_DBG_SOURCE;
        LLGL_DBG_ERROR(ErrorType::InvalidState, "attempt to disassemble uncompiled shader code");
    }
    return instance.Disassemble(flags);
}

std::string DbgShader::QueryInfoLog()
{
    return instance.QueryInfoLog();
}


} // /namespace LLGL



// ================================================================================
