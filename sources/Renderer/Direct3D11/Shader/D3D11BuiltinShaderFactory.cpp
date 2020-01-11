/*
 * D3D11BuiltinShaderFactory.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11BuiltinShaderFactory.h"
#include "Builtin/D3D11Builtin.h"
#include "../../DXCommon/DXCore.h"
#include "../../DXCommon/DXTypes.h"
#include <stdexcept>


namespace LLGL
{


D3D11BuiltinShaderFactory& D3D11BuiltinShaderFactory::Get()
{
    static D3D11BuiltinShaderFactory instance;
    return instance;
}

void D3D11BuiltinShaderFactory::CreateBuiltinShaders(ID3D11Device* device)
{
    LoadBuiltinShader(device, D3D11BuiltinShader::CopyTexture1DFromBufferCS, LLGL_IDR_D3D11_COPYTEXTURE1DFROMBUFFER_CS);
    LoadBuiltinShader(device, D3D11BuiltinShader::CopyTexture2DFromBufferCS, LLGL_IDR_D3D11_COPYTEXTURE2DFROMBUFFER_CS);
    LoadBuiltinShader(device, D3D11BuiltinShader::CopyTexture3DFromBufferCS, LLGL_IDR_D3D11_COPYTEXTURE3DFROMBUFFER_CS);
    LoadBuiltinShader(device, D3D11BuiltinShader::CopyBufferFromTexture1DCS, LLGL_IDR_D3D11_COPYBUFFERFROMTEXTURE1D_CS);
    LoadBuiltinShader(device, D3D11BuiltinShader::CopyBufferFromTexture2DCS, LLGL_IDR_D3D11_COPYBUFFERFROMTEXTURE2D_CS);
    LoadBuiltinShader(device, D3D11BuiltinShader::CopyBufferFromTexture3DCS, LLGL_IDR_D3D11_COPYBUFFERFROMTEXTURE3D_CS);
}

void D3D11BuiltinShaderFactory::Clear()
{
    for (auto& native : builtinShaders_)
        native.vs = nullptr;
}

const D3D11NativeShader& D3D11BuiltinShaderFactory::GetBulitinShader(const D3D11BuiltinShader builtin) const
{
    const auto idx = static_cast<std::size_t>(builtin);
    if (idx < D3D11BuiltinShaderFactory::g_numBuiltinShaders)
        return builtinShaders_[idx];
    else
        return builtinShaders_[0];
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

void D3D11BuiltinShaderFactory::LoadBuiltinShader(ID3D11Device* device, const D3D11BuiltinShader builtin, int resourceID)
{
    if (auto blob = DXCreateBlobFromResource(resourceID))
    {
        const auto idx = static_cast<std::size_t>(builtin);
        builtinShaders_[idx] = D3D11Shader::CreateNativeShaderFromBlob(device, GetBuiltinShaderType(builtin), blob.Get());
    }
    else
        throw std::runtime_error("failed to load builtin D3D11 shader resource");
}


} // /namespace LLGL



// ================================================================================
