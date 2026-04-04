/*
 * WGShader.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "WGShader.h"
#include "../../../Core/Assertion.h"


namespace LLGL
{


WGShader::WGShader(const ShaderDescriptor& desc) :
    Shader { desc.type }
{
}

const Report* WGShader::GetReport() const
{
    return nullptr; //todo
}

bool WGShader::Reflect(ShaderReflection& reflection) const
{
    return false; //todo
}


} // /namespace LLGL



// ================================================================================
