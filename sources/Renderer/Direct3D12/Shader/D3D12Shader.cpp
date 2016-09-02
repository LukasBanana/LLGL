/*
 * D3D12Shader.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12Shader.h"


namespace LLGL
{


D3D12Shader::D3D12Shader(const ShaderType type) :
    Shader( type )
{
}

bool D3D12Shader::Compile(const std::string& shaderSource)
{
    return false;
}

std::string D3D12Shader::QueryInfoLog()
{
    return "";
}

D3D12_SHADER_BYTECODE D3D12Shader::GetByteCode() const
{
    D3D12_SHADER_BYTECODE byteCode;

    byteCode.pShaderBytecode    = byteCode_.data();
    byteCode.BytecodeLength     = static_cast<SIZE_T>(byteCode_.size());

    return byteCode;
}


} // /namespace LLGL



// ================================================================================
