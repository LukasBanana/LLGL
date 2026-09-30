/*
 * NullShader.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "NullShader.h"


namespace LLGL
{


NullShader::NullShader(const ShaderDescriptor& desc) :
    Shader { desc.type },
    desc   { desc      }
{
    if (desc.debugName != nullptr)
        SetDebugName(desc.debugName);
}

void NullShader::SetDebugName(const char* name)
{
    if (name != nullptr)
        label_ = name;
    else
        label_.clear();
}

const Report* NullShader::GetReport() const
{
    return nullptr; // dummy
}

bool NullShader::Reflect(ShaderReflection& reflection) const
{
    return false; // dummy
}


} // /namespace LLGL



// ================================================================================
