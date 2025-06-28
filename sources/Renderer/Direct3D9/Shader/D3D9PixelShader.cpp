/*
 * D3D9PixelShader.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D9PixelShader.h"
#include "../D3D9Core.h"


namespace LLGL
{


D3D9PixelShader::D3D9PixelShader(IDirect3DDevice9* device, const ShaderDescriptor& shaderDesc) :
    D3D9Shader { ShaderType::Fragment}
{
    BuildShader(device, shaderDesc);
}

bool D3D9PixelShader::Reflect(ShaderReflection& reflection) const
{
    return false; //TODO
}


/*
 * ======= Private: =======
 */

HRESULT D3D9PixelShader::CreateD3DShaderFromBlob(IDirect3DDevice9* device, ID3DBlob* byteCode)
{
    return device->CreatePixelShader(static_cast<const DWORD*>(byteCode->GetBufferPointer()), d3dShader_.ReleaseAndGetAddressOf());
}


} // /namespace LLGL



// ================================================================================
