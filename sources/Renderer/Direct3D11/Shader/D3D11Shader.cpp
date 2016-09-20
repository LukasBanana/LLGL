/*
 * D3D11Shader.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11Shader.h"
#include "../../DXCommon/DXCore.h"
#include <algorithm>
#include <stdexcept>
#include <d3dcompiler.h>


namespace LLGL
{


D3D11Shader::D3D11Shader(ID3D11Device* device, const ShaderType type) :
    Shader  ( type   ),
    device_ ( device )
{
}

// see https://msdn.microsoft.com/en-us/library/windows/desktop/dd607324(v=vs.85).aspx
bool D3D11Shader::Compile(const ShaderSource& shaderSource)
{
    /* Get parameter from union */
    const auto& sourceCode  = shaderSource.sourceHLSL.sourceCode;
    const auto& entryPoint  = shaderSource.sourceHLSL.entryPoint;
    const auto& target      = shaderSource.sourceHLSL.target;
    auto flags              = shaderSource.sourceHLSL.flags;

    /* Compile shader code */
    ComPtr<ID3DBlob> code;

    auto hr = D3DCompile(
        sourceCode.data(),
        sourceCode.size(),
        nullptr,                                                // LPCSTR               pSourceName
        nullptr,                                                // D3D_SHADER_MACRO*    pDefines
        nullptr,                                                // ID3DInclude*         pInclude
        (entryPoint.empty() ? nullptr : entryPoint.c_str()),    // LPCSTR               pEntrypoint
        target.c_str(),                                         // LPCSTR               pTarget
        DXGetCompilerFlags(flags),                              // UINT                 Flags1
        0,                                                      // UINT                 Flags2 (recommended to always be 0)
        &code,                                                  // ID3DBlob**           ppCode
        &errors_                                                // ID3DBlob**           ppErrorMsgs
    );

    /* Get byte code from blob */
    if (code)
    {
        byteCode_ = DXGetBlobData(code.Get());
        CreateHardwareShader(nullptr);
    }

    if (FAILED(hr))
        return false;

    /* Get shader reflection */
    ReflectShader();

    return true;
}

std::string D3D11Shader::Disassemble(int flags)
{
    if (!byteCode_.empty())
    {
        ComPtr<ID3DBlob> disasm;

        auto hr = D3DDisassemble(byteCode_.data(), byteCode_.size(), DXGetDisassemblerFlags(flags), nullptr, &disasm);
        DXThrowIfFailed(hr, "failed to disassemble D3D11 shader byte code");

        return DXGetBlobString(disasm.Get());
    }
    return "";
}

std::string D3D11Shader::QueryInfoLog()
{
    return (errors_.Get() != nullptr ? DXGetBlobString(errors_.Get()) : "");
}


/*
 * ======= Private: =======
 */

void D3D11Shader::CreateHardwareShader(ID3D11ClassLinkage* classLinkage)
{
    HRESULT hr = 0;

    switch (GetType())
    {
        case ShaderType::Vertex:
            hr = device_->CreateVertexShader(byteCode_.data(), byteCode_.size(), classLinkage, &hardwareShader_.vs);
            DXThrowIfFailed(hr, "failed to create D3D11 vertex shader");
            break;
        case ShaderType::Fragment:
            hr = device_->CreatePixelShader(byteCode_.data(), byteCode_.size(), classLinkage, &hardwareShader_.ps);
            DXThrowIfFailed(hr, "failed to create D3D11 pixel shader");
            break;
        case ShaderType::TessControl:
            hr = device_->CreateHullShader(byteCode_.data(), byteCode_.size(), classLinkage, &hardwareShader_.hs);
            DXThrowIfFailed(hr, "failed to create D3D11 hull shader");
            break;
        case ShaderType::TessEvaluation:
            hr = device_->CreateDomainShader(byteCode_.data(), byteCode_.size(), classLinkage, &hardwareShader_.ds);
            DXThrowIfFailed(hr, "failed to create D3D11 domain shader");
            break;
        case ShaderType::Geometry:
            hr = device_->CreateGeometryShader(byteCode_.data(), byteCode_.size(), classLinkage, &hardwareShader_.gs);
            DXThrowIfFailed(hr, "failed to create D3D11 geometry shader");
            break;
        case ShaderType::Compute:
            hr = device_->CreateComputeShader(byteCode_.data(), byteCode_.size(), classLinkage, &hardwareShader_.cs);
            DXThrowIfFailed(hr, "failed to create D3D11 compute shader");
            break;
    }
}

void D3D11Shader::ReflectShader()
{
    HRESULT hr = 0;

    /* Get shader reflection */
    ComPtr<ID3D11ShaderReflection> reflection;
    hr = D3DReflect(byteCode_.data(), byteCode_.size(), IID_PPV_ARGS(&reflection));
    DXThrowIfFailed(hr, "failed to retrieve D3D11 shader reflection");

    D3D11_SHADER_DESC shaderDesc;
    hr = reflection->GetDesc(&shaderDesc);
    DXThrowIfFailed(hr, "failed to retrieve D3D11 shader descriptor");

    /* Get input parameter descriptors */
    if (GetType() == ShaderType::Vertex)
    {
        for (UINT i = 0; i < shaderDesc.InputParameters; ++i)
        {
            /* Get signature parameter descriptor */
            D3D11_SIGNATURE_PARAMETER_DESC paramDesc;
            hr = reflection->GetInputParameterDesc(i, &paramDesc);
            DXThrowIfFailed(
                hr, "failed to retrieve D3D11 signature parameter descriptor (" +
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

        D3D11_SHADER_BUFFER_DESC shaderBufferDesc;
        hr = constBufferReflection->GetDesc(&shaderBufferDesc);
        DXThrowIfFailed(hr, "failed to retrieve D3D11 shader buffer descriptor");

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
        D3D11_SHADER_INPUT_BIND_DESC inputBindDesc;
        hr = reflection->GetResourceBindingDesc(i, &inputBindDesc);
        DXThrowIfFailed(hr, "failed to retrieve D3D11 shader input binding descriptor");

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
