/*
 * DbgShader.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "DbgShader.h"
#include "DbgCore.h"


namespace LLGL
{


DbgShader::DbgShader(Shader& instance, const ShaderDescriptor& desc) :
    Shader    { desc.type },
    desc      { desc      },
    instance  { instance  }
{
}

void DbgShader::SetName(const char* name)
{
    DbgSetObjectName(*this, name);
}

bool DbgShader::HasErrors() const
{
    return instance.HasErrors();
}

std::string DbgShader::GetReport() const
{
    return instance.GetReport();
}


} // /namespace LLGL



// ================================================================================
