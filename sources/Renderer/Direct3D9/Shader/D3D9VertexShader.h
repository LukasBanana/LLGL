/*
 * D3D9VertexShader.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D9_VERTEX_SHADER_H
#define LLGL_D3D9_VERTEX_SHADER_H


#include "D3D9Shader.h"
#include <LLGL/Container/ArrayView.h>
#include <LLGL/Container/SmallVector.h>


namespace LLGL
{


// Stream source frequency for instanced drawing.
struct D3D9StreamSourceFreq
{
    UINT stream;
    UINT divider;
};

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

        // Returns the stream source frequency dividers for instanced data, i.e. all the values for streams starting at 1,
        // since stream 0 is reserved for D3DSTREAMSOURCE_INDEXEDDATA and the number of instances to draw.
        inline const SmallVector<D3D9StreamSourceFreq, 1>& GetStreamSourceFreq() const
        {
            return streamSourceFreq_;
        }

    private:

        HRESULT CreateD3DShaderFromBlob(IDirect3DDevice9* device, ID3DBlob* byteCode) override;

    private:

        void BuildVertexDeclaration(IDirect3DDevice9* device, ArrayView<VertexAttribute> vertexAttribs);
        void BuildStreamSourceFreq(ArrayView<VertexAttribute> vertexAttribs);

    private:

        ComPtr<IDirect3DVertexShader9>          d3dShader_;
        ComPtr<IDirect3DVertexDeclaration9>     d3dVertexDecl_;
        SmallVector<D3D9StreamSourceFreq, 1>    streamSourceFreq_;

};


} // /namespace LLGL


#endif



// ================================================================================
