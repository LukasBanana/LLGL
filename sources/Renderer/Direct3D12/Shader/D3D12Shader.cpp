/*
 * D3D12Shader.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12Shader.h"
#include "../DXCore.h"
#include <algorithm>
#include <stdexcept>
#include <d3dcompiler.h>


namespace LLGL
{


D3D12Shader::D3D12Shader(const ShaderType type) :
    Shader( type )
{
}

D3D12Shader::~D3D12Shader()
{
    SafeRelease(errors_);
}

bool D3D12Shader::Compile(const std::string& shaderSource)
{
    throw std::runtime_error("invalid 'Shader::Compile' function for HLSL");
}

bool D3D12Shader::Compile(const std::string& shaderSource, const std::string& entryPoint, const std::string& target)
{
    SafeRelease(errors_);

    UINT compileFlags = 0;

    ID3DBlob* code = nullptr;

    auto hr = D3DCompile(
        shaderSource.data(),
        shaderSource.size(),
        nullptr,                                                // LPCSTR               pSourceName
        nullptr,                                                // D3D_SHADER_MACRO*    pDefines
        nullptr,                                                // ID3DInclude*         pInclude
        (entryPoint.empty() ? nullptr : entryPoint.c_str()),    // LPCSTR               pEntrypoint
        target.c_str(),                                         // LPCSTR               pTarget
        compileFlags,                                           // UINT                 Flags1
        0,                                                      // UINT                 Flags2 (recommended to always be 0)
        &code,                                                  // ID3DBlob**           ppCode
        &errors_                                                // ID3DBlob**           ppErrorMsgs
    );

    DXThrowIfFailed(hr, "failed to compile HLSL code with 'D3DCompile' function");

    return true;
}

std::string D3D12Shader::QueryInfoLog()
{
    return (errors_ != nullptr ? DXGetBlobString(errors_) : "");
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
