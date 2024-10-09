/*
 * D3D11VertexShader.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D11_VERTEX_SHADER_H
#define LLGL_D3D11_VERTEX_SHADER_H


#include "D3D11Shader.h"


namespace LLGL
{


// The vertex shader has its own implementation to store additional information,
// input-layout and an optional proxy geometry-shader for stream-outputs that is.
class D3D11VertexShader final : public D3D11Shader
{

    public:

        D3D11VertexShader(ID3D11Device* device, const ShaderDescriptor& desc);

        // Returns the input layout for vertex shaders.
        inline const ComPtr<ID3D11InputLayout>& GetInputLayout() const
        {
            return inputLayout_;
        }

        // Returns the proxy geometry shader for stream-output if there is one.
        inline const ComPtr<ID3D11GeometryShader>& GetProxyGeometryShader() const
        {
            return proxyGeomtryShader_;
        }

    private:

        void BuildInputLayout(ID3D11Device* device, UINT numVertexAttribs, const VertexAttribute* vertexAttribs);

    private:

        ComPtr<ID3D11InputLayout>       inputLayout_;
        ComPtr<ID3D11GeometryShader>    proxyGeomtryShader_;

};


} // /namespace LLGL


#endif



// ================================================================================
