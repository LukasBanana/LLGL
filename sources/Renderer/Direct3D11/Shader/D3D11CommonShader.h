/*
 * D3D11CommonShader.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D11_COMMON_SHADER_H
#define LLGL_D3D11_COMMON_SHADER_H


#include "D3D11Shader.h"


namespace LLGL
{


class D3D11CommonShader final : public D3D11Shader
{

    public:

        D3D11CommonShader(ID3D11Device* device, const ShaderDescriptor& desc);

};


} // /namespace LLGL


#endif



// ================================================================================
