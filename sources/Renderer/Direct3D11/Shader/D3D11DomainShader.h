/*
 * D3D11DomainShader.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D11_DOMAIN_SHADER_H
#define LLGL_D3D11_DOMAIN_SHADER_H


#include "D3D11Shader.h"


namespace LLGL
{


// The domain shader has its own implementation to store additional information,
// optional proxy geometry-shader for stream-outputs that is.
class D3D11DomainShader final : public D3D11Shader
{

    public:

        D3D11DomainShader(ID3D11Device* device, const ShaderDescriptor& desc);

        // Returns the proxy geometry shader for stream-output if there is one.
        inline const ComPtr<ID3D11GeometryShader>& GetProxyGeometryShader() const
        {
            return proxyGeomtryShader_;
        }

    private:

        ComPtr<ID3D11GeometryShader> proxyGeomtryShader_;

};


} // /namespace LLGL


#endif



// ================================================================================
