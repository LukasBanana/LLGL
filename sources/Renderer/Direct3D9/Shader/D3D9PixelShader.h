/*
 * D3D9PixelShader.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D9_PIXEL_SHADER_H
#define LLGL_D3D9_PIXEL_SHADER_H


#include "D3D9Shader.h"


namespace LLGL
{


class D3D9PixelShader final : public D3D9Shader
{

    public:

        bool Reflect(ShaderReflection& reflection) const override;

    public:

        D3D9PixelShader(IDirect3DDevice9* device, const ShaderDescriptor& shaderDesc);

        inline IDirect3DPixelShader9* GetNative() const
        {
            return d3dShader_.Get();
        }

    private:

        HRESULT CreateD3DShaderFromBlob(IDirect3DDevice9* device, ID3DBlob* byteCode) override;

    private:

        ComPtr<IDirect3DPixelShader9> d3dShader_;

};


} // /namespace LLGL


#endif



// ================================================================================
