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

bool D3D12Shader::Compile(const std::string& shaderSource)
{
    throw std::runtime_error("invalid 'Shader::Compile' function for HLSL");
}

// see https://msdn.microsoft.com/en-us/library/windows/desktop/gg615083(v=vs.85).aspx
static UINT GetDXCompileFlags(int flags)
{
    UINT dxFlags = 0;

    if ((flags & ShaderCompileFlags::Debug) != 0)
        dxFlags |= D3DCOMPILE_DEBUG;

    if ((flags & ShaderCompileFlags::O1) != 0)
        dxFlags |= D3DCOMPILE_OPTIMIZATION_LEVEL1;
    else if ((flags & ShaderCompileFlags::O2) != 0)
        dxFlags |= D3DCOMPILE_OPTIMIZATION_LEVEL2;
    else if ((flags & ShaderCompileFlags::O3) != 0)
        dxFlags |= D3DCOMPILE_OPTIMIZATION_LEVEL3;
    else
        dxFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;//D3DCOMPILE_OPTIMIZATION_LEVEL0;

    if ((flags & ShaderCompileFlags::WarnError) != 0)
        dxFlags |= D3DCOMPILE_WARNINGS_ARE_ERRORS;

    return dxFlags;
}

// see https://msdn.microsoft.com/en-us/library/windows/desktop/dd607324(v=vs.85).aspx
bool D3D12Shader::Compile(const std::string& shaderSource, const std::string& entryPoint, const std::string& target, int flags)
{
    /* Compile shader code */
    ComPtr<ID3DBlob> code;

    auto hr = D3DCompile(
        shaderSource.data(),
        shaderSource.size(),
        nullptr,                                                // LPCSTR               pSourceName
        nullptr,                                                // D3D_SHADER_MACRO*    pDefines
        nullptr,                                                // ID3DInclude*         pInclude
        (entryPoint.empty() ? nullptr : entryPoint.c_str()),    // LPCSTR               pEntrypoint
        target.c_str(),                                         // LPCSTR               pTarget
        GetDXCompileFlags(flags),                               // UINT                 Flags1
        0,                                                      // UINT                 Flags2 (recommended to always be 0)
        &code,                                                  // ID3DBlob**           ppCode
        &errors_                                                // ID3DBlob**           ppErrorMsgs
    );

    /* Get byte code from blob */
    if (code)
        byteCode_ = DXGetBlobData(code.Get());

    if (FAILED(hr))
        return false;

    /* Get shader reflection */
    ReflectShader();

    return true;
}

// see https://msdn.microsoft.com/en-us/library/windows/desktop/dd607326(v=vs.85).aspx
static UINT GetDXDisassembleFlags(int flags)
{
    UINT dxFlags = 0;

    if ((flags & ShaderDisassembleFlags::InstructionOnly) != 0)
        dxFlags |= D3D_DISASM_INSTRUCTION_ONLY;

    return dxFlags;
}

std::string D3D12Shader::Disassemble(int flags)
{
    if (!byteCode_.empty())
    {
        ComPtr<ID3DBlob> disasm;

        auto hr = D3DDisassemble(byteCode_.data(), byteCode_.size(), GetDXDisassembleFlags(flags), nullptr, &disasm);
        DXThrowIfFailed(hr, "failed to disassemble D3D12 shader byte code");

        return DXGetBlobString(disasm.Get());
    }
    return "";
}

std::string D3D12Shader::QueryInfoLog()
{
    return (errors_.Get() != nullptr ? DXGetBlobString(errors_.Get()) : "");
}

D3D12_SHADER_BYTECODE D3D12Shader::GetByteCode() const
{
    D3D12_SHADER_BYTECODE byteCode;

    byteCode.pShaderBytecode    = byteCode_.data();
    byteCode.BytecodeLength     = static_cast<SIZE_T>(byteCode_.size());

    return byteCode;
}


/*
 * ======= Private: =======
 */

void D3D12Shader::ReflectShader()
{
    HRESULT hr = 0;

    /* Get shader reflection */
    ComPtr<ID3D12ShaderReflection> reflection;
    hr = D3DReflect(byteCode_.data(), byteCode_.size(), IID_PPV_ARGS(&reflection));
    DXThrowIfFailed(hr, "failed to retrieve D3D12 shader reflection");

    D3D12_SHADER_DESC shaderDesc;
    hr = reflection->GetDesc(&shaderDesc);
    DXThrowIfFailed(hr, "failed to retrieve D3D12 shader descriptor");

    /* Get input parameter descriptors */
    if (GetType() == ShaderType::Vertex)
    {
        for (UINT i = 0; i < shaderDesc.InputParameters; ++i)
        {
            /* Get signature parameter descriptor */
            D3D12_SIGNATURE_PARAMETER_DESC paramDesc;
            hr = reflection->GetInputParameterDesc(i, &paramDesc);
            DXThrowIfFailed(
                hr, "failed to retrieve D3D12 signature parameter descriptor (" +
                std::to_string(i + 1) + " of " + std::to_string(shaderDesc.InputParameters) + ")"
            );

            /* Add vertex attribute to output list */
            VertexAttribute vertexAttrib;
            {
                vertexAttrib.name           = std::string(paramDesc.SemanticName);
                vertexAttrib.semanticIndex  = paramDesc.SemanticIndex;
            }
            vertexAttributes_.push_back(vertexAttrib);
        }
    }

    /* Get constant buffer descriptors */
    unsigned int bufferIdx = 0;
    for (UINT i = 0; i < shaderDesc.ConstantBuffers; ++i)
    {
        /* Get shader buffer descriptor */
        auto constBufferReflection = reflection->GetConstantBufferByIndex(i);

        D3D12_SHADER_BUFFER_DESC shaderBufferDesc;
        hr = constBufferReflection->GetDesc(&shaderBufferDesc);
        DXThrowIfFailed(hr, "failed to retrieve D3D12 shader buffer descriptor");

        if (shaderBufferDesc.Type == D3D_CT_CBUFFER)
        {
            /* Add constant buffer descriptor to output list */
            ConstantBufferDescriptor constBufferDesc;
            {
                constBufferDesc.name    = std::string(shaderBufferDesc.Name);
                constBufferDesc.index   = bufferIdx++;
                constBufferDesc.size    = shaderBufferDesc.Size;
            }
            constantBufferDescs_.push_back(constBufferDesc);
        }
    }

    /* Get storage buffer descriptors */
    bufferIdx = 0;
    for (UINT i = 0; i < shaderDesc.BoundResources; ++i)
    {
        /* Get shader input resource descriptor */
        D3D12_SHADER_INPUT_BIND_DESC inputBindDesc;
        hr = reflection->GetResourceBindingDesc(i, &inputBindDesc);
        DXThrowIfFailed(hr, "failed to retrieve D3D12 shader input binding descriptor");

        if ( inputBindDesc.Type >= D3D_SIT_UAV_RWTYPED &&
             inputBindDesc.Type <= D3D_SIT_UAV_RWSTRUCTURED_WITH_COUNTER )
        {
            /* Add storage buffer descriptor to output list */
            StorageBufferDescriptor storeBufferDesc;
            {
                storeBufferDesc.name    = std::string(inputBindDesc.Name);
                storeBufferDesc.index   = bufferIdx++;
            }
            storageBufferDescs_.push_back(storeBufferDesc);
        }
    }
}


} // /namespace LLGL



// ================================================================================
