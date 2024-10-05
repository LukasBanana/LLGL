/*
 * D3D11BuiltinShaderFactory.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D11BuiltinShaderFactory.h"
#include "Builtin/D3D11Builtin.h"
#include "../../DXCommon/DXCore.h"
#include "../../DXCommon/DXTypes.h"
#include "../../../Core/Exception.h"


namespace LLGL
{


D3D11BuiltinShaderFactory& D3D11BuiltinShaderFactory::Get()
{
    static D3D11BuiltinShaderFactory instance;
    return instance;
}

void D3D11BuiltinShaderFactory::CreateBuiltinShaders(ID3D11Device* device)
{
    LoadBuiltinShader(device, D3D11BuiltinShader::CopyTexture1DFromBufferCS,
        LLGL_IDR_D3D11_COPYTEXTURE1DFROMBUFFER_CS, sizeof(LLGL_IDR_D3D11_COPYTEXTURE1DFROMBUFFER_CS));
    LoadBuiltinShader(device, D3D11BuiltinShader::CopyTexture2DFromBufferCS,
        LLGL_IDR_D3D11_COPYTEXTURE2DFROMBUFFER_CS, sizeof(LLGL_IDR_D3D11_COPYTEXTURE2DFROMBUFFER_CS));
    LoadBuiltinShader(device, D3D11BuiltinShader::CopyTexture3DFromBufferCS,
        LLGL_IDR_D3D11_COPYTEXTURE3DFROMBUFFER_CS, sizeof(LLGL_IDR_D3D11_COPYTEXTURE3DFROMBUFFER_CS));
    LoadBuiltinShader(device, D3D11BuiltinShader::CopyBufferFromTexture1DCS,
        LLGL_IDR_D3D11_COPYBUFFERFROMTEXTURE1D_CS, sizeof(LLGL_IDR_D3D11_COPYBUFFERFROMTEXTURE1D_CS));
    LoadBuiltinShader(device, D3D11BuiltinShader::CopyBufferFromTexture2DCS,
        LLGL_IDR_D3D11_COPYBUFFERFROMTEXTURE2D_CS, sizeof(LLGL_IDR_D3D11_COPYBUFFERFROMTEXTURE2D_CS));
    LoadBuiltinShader(device, D3D11BuiltinShader::CopyBufferFromTexture3DCS,
        LLGL_IDR_D3D11_COPYBUFFERFROMTEXTURE3D_CS, sizeof(LLGL_IDR_D3D11_COPYBUFFERFROMTEXTURE3D_CS));
}

void D3D11BuiltinShaderFactory::Clear()
{
    for (ComPtr<ID3D11ComputeShader>& native : builtinComputeShaders_)
        native.Reset();
}

ID3D11ComputeShader* D3D11BuiltinShaderFactory::GetBulitinComputeShader(const D3D11BuiltinShader builtin) const
{
    const std::size_t idx = static_cast<std::size_t>(builtin);
    if (idx < D3D11BuiltinShaderFactory::numBuiltinShaders)
        return builtinComputeShaders_[idx].Get();
    else
        return nullptr;
}

static ShaderType GetBuiltinShaderType(const D3D11BuiltinShader builtin)
{
    switch (builtin)
    {
        case D3D11BuiltinShader::CopyTexture1DFromBufferCS:
        case D3D11BuiltinShader::CopyTexture2DFromBufferCS:
        case D3D11BuiltinShader::CopyTexture3DFromBufferCS:
        case D3D11BuiltinShader::CopyBufferFromTexture1DCS:
        case D3D11BuiltinShader::CopyBufferFromTexture2DCS:
        case D3D11BuiltinShader::CopyBufferFromTexture3DCS:
            return ShaderType::Compute;
    }
    return ShaderType::Undefined;
}

void D3D11BuiltinShaderFactory::LoadBuiltinShader(
    ID3D11Device*               device,
    const D3D11BuiltinShader    builtin,
    const BYTE*                 shaderBytecode,
    size_t                      shaderBytecodeSize)
{
    if (ComPtr<ID3DBlob> blob = DXCreateBlob(shaderBytecode, shaderBytecodeSize))
    {
        const std::size_t idx = static_cast<std::size_t>(builtin);
        D3D11Shader::CreateNativeShaderFromBlob(device, GetBuiltinShaderType(builtin), blob.Get()).As(&builtinComputeShaders_[idx]);
    }
    else
        LLGL_TRAP("failed to load builtin D3D11 shader resource");
}


} // /namespace LLGL



// ================================================================================
