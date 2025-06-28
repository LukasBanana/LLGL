/*
 * D3D9VertexShader.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D9_VERTEX_SHADER_H
#define LLGL_D3D9_VERTEX_SHADER_H


#include "D3D9Shader.h"
#include "../../DXCommon/ComPtr.h"
#include <LLGL/Container/ArrayView.h>


namespace LLGL
{


class D3D9VertexShader final : public D3D9Shader
{

    public:

        bool Reflect(ShaderReflection& reflection) const override;

    public:

        D3D9VertexShader(IDirect3DDevice9* device, const ShaderDescriptor& shaderDesc);

        inline IDirect3DVertexShader9* GetNative() const
        {
            return d3dShader_.Get();
        }

        inline IDirect3DVertexDeclaration9* GetVertexDeclaration() const
        {
            return d3dVertexDecl_.Get();
        }

    private:

        HRESULT CreateD3DShaderFromBlob(IDirect3DDevice9* device, ID3DBlob* byteCode) override;

    private:

        void BuildVertexDeclaration(IDirect3DDevice9* device, const ArrayView<VertexAttribute>& vertexAttribs);

    private:

        ComPtr<IDirect3DVertexShader9>      d3dShader_;
        ComPtr<IDirect3DVertexDeclaration9> d3dVertexDecl_;

};


} // /namespace LLGL


#endif



// ================================================================================
