/*
 * D3D11CommonShader.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D11CommonShader.h"


namespace LLGL
{


D3D11CommonShader::D3D11CommonShader(ID3D11Device* device, const ShaderDescriptor& desc) :
    D3D11Shader { desc.type }
{
    BuildShader(device, desc);
    if (desc.debugName != nullptr)
        SetDebugName(desc.debugName);
}


} // /namespace LLGL



// ================================================================================
