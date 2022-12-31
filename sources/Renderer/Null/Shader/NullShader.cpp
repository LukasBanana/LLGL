/*
 * NullShader.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "NullShader.h"


namespace LLGL
{


NullShader::NullShader(const ShaderDescriptor& desc) :
    Shader { desc.type },
    desc   { desc      }
{
}

void NullShader::SetName(const char* name)
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
    switch (GetType())
    {
        case ShaderType::Vertex:
            reflection.vertex = desc.vertex;
            break;
        case ShaderType::Fragment:
            reflection.fragment = desc.fragment;
            break;
        case ShaderType::Compute:
            reflection.compute = desc.compute;
            break;
        default:
            break;
    }
    return true;
}


} // /namespace LLGL



// ================================================================================
