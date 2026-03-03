/*
 * D3D9Shader.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D9Shader.h"
#include "D3D9ConstantTableParser.h"
#include "../D3D9Core.h"
#include "../../DXCommon/ComPtr.h"
#include "../../../Core/Assertion.h"
#include "../../../Core/CoreUtils.h"
#include "../../../Core/StringUtils.h"
#include "../../../Core/ReportUtils.h"
#include <LLGL/Utils/TypeNames.h>
#include <LLGL/Utils/ForRange.h>
#include <d3dcompiler.h>


namespace LLGL
{


D3D9Shader::D3D9Shader(const ShaderType type) :
    Shader { type }
{
}

void D3D9Shader::SetDebugName(const char* name)
{
    // dummy
}

const Report* D3D9Shader::GetReport() const
{
    return (report_ ? &report_ : nullptr);
}


/*
 * ======= Protected: =======
 */

bool D3D9Shader::BuildShader(IDirect3DDevice9* device, const ShaderDescriptor& shaderDesc)
{
    if (IsShaderSourceCode(shaderDesc.sourceType))
        return CompileSource(device, shaderDesc);
    else
        return LoadBinary(device, shaderDesc);
}


/*
 * ======= Private: =======
 */

// see https://msdn.microsoft.com/en-us/library/windows/desktop/dd607324(v=vs.85).aspx
bool D3D9Shader::CompileSource(IDirect3DDevice9* device, const ShaderDescriptor& shaderDesc)
{
    /* Get source code */
    std::string fileContent;
    const char* sourceCode      = nullptr;
    SIZE_T      sourceLength    = 0;
    const char* sourceName      = nullptr;

    if (shaderDesc.sourceType == ShaderSourceType::CodeFile)
    {
        fileContent     = ReadFileString(shaderDesc.source);
        sourceCode      = fileContent.c_str();
        sourceLength    = fileContent.size();
        sourceName      = shaderDesc.debugName != nullptr ? shaderDesc.debugName : shaderDesc.source;
    }
    else
    {
        sourceCode      = shaderDesc.source;
        sourceLength    = shaderDesc.sourceSize;
        sourceName      = shaderDesc.debugName;
    }

    /* If 'sourceSize' is 0, the source length is determined from the NUL-terminated source string */
    if (sourceLength == 0 && sourceCode != nullptr)
        sourceLength = std::strlen(sourceCode);

    /* Get parameters from shader descriptor */
    const char* entry   = shaderDesc.entryPoint;
    const char* target  = (shaderDesc.profile != nullptr ? shaderDesc.profile : "");
    auto*       defines = reinterpret_cast<const D3D_SHADER_MACRO*>(shaderDesc.defines);
    long        flags   = shaderDesc.flags;

    /* Compile shader code */
    ComPtr<ID3DBlob> errors;
    ComPtr<ID3DBlob> byteCode;

    HRESULT hr = D3DCompile(
        sourceCode,
        sourceLength,
        sourceName,                         // LPCSTR               pSourceName
        defines,                            // D3D_SHADER_MACRO*    pDefines
        D3D_COMPILE_STANDARD_FILE_INCLUDE,  // ID3DInclude*         pInclude
        entry,                              // LPCSTR               pEntrypoint
        target,                             // LPCSTR               pTarget
        D3DGetFxcCompilerFlags(flags),      // UINT                 Flags1
        0,                                  // UINT                 Flags2 (recommended to always be 0)
        byteCode.ReleaseAndGetAddressOf(),  // ID3DBlob**           ppCode
        errors.ReleaseAndGetAddressOf()     // ID3DBlob**           ppErrorMsgs
    );

    /* Get byte code from blob */
    if (byteCode)
    {
        HRESULT hr = CreateD3DShaderFromBlob(device, byteCode.Get());
        if (FAILED(hr))
        {
            report_.Errorf("Failed to create %s shader for D3D9 backend (error=%s)\n", ToString(shaderDesc.type), D3DErrorToStrOrHex(hr));
            return false;
        }
        if (ReflectConstantTable(byteCode.Get()))
            return false;
    }

    /* Store if compilation was successful */
    const bool hasErrors = FAILED(hr);
    ResetReportWithNewline(report_, D3DGetBlobString(errors.Get()), hasErrors);
    return !hasErrors;
}

bool D3D9Shader::LoadBinary(IDirect3DDevice9* device, const ShaderDescriptor& shaderDesc)
{
    ComPtr<ID3DBlob> byteCode;

    if (shaderDesc.sourceType == ShaderSourceType::BinaryFile)
    {
        /* Load binary code from file */
        byteCode = D3DCreateBlob(ReadFileBuffer(shaderDesc.source));
    }
    else
    {
        /* Copy binary code into container and create native shader */
        byteCode = D3DCreateBlob(shaderDesc.source, shaderDesc.sourceSize);
    }

    if (byteCode.Get() != nullptr && byteCode->GetBufferSize() > 0)
    {
        /* Create native shader object */
        HRESULT hr = CreateD3DShaderFromBlob(device, byteCode.Get());
        if (FAILED(hr))
        {
            report_.Errorf("Failed to create %s shader for D3D9 backend (error=%s)\n", ToString(shaderDesc.type), D3DErrorToStrOrHex(hr));
            return false;
        }
        return ReflectConstantTable(byteCode.Get());
    }

    report_.Errorf("%s shader error: missing DXBC bytecode\n", ToString(shaderDesc.type));
    return false;
}

bool D3D9Shader::ReflectConstantTable(ID3DBlob* byteCode)
{
    HRESULT hr = D3DParseSM3ConstantTable(byteCode->GetBufferPointer(), constantTable_);
    if (FAILED(hr))
    {
        report_.Errorf("Failed to reflect constant table for D3D9 shader (error=%s)", D3DErrorToStrOrHex(hr));
        return false;
    }
    return true;
}


} // /namespace LLGL



// ================================================================================
