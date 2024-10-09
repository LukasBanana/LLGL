/*
 * D3D11DomainShader.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D11DomainShader.h"


namespace LLGL
{


D3D11DomainShader::D3D11DomainShader(ID3D11Device* device, const ShaderDescriptor& desc) :
    D3D11Shader { desc.type }
{
    if (BuildShader(device, desc))
    {
        /* Build optional proxy geometry shader if there are any output attributes */
        if (!desc.vertex.outputAttribs.empty())
            BuildProxyGeometryShader(device, desc, proxyGeomtryShader_);
    }
    if (desc.debugName != nullptr)
        SetDebugName(desc.debugName);
}


} // /namespace LLGL



// ================================================================================
