/*
 * CsShaderProgram.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "CsShaderProgram.h"
#include "CsHelper.h"


namespace LHermanns
{

namespace SharpLLGL
{


ShaderProgram::ShaderProgram(::LLGL::ShaderProgram* native) :
    native_ { native }
{
}

::LLGL::ShaderProgram* ShaderProgram::Native::get()
{
    return native_;
}

bool ShaderProgram::HasErrors::get()
{
    return native_->HasErrors();
}

String^ ShaderProgram::QueryInfoLog()
{
    auto info = native_->QueryInfoLog();
    return gcnew String(ToManagedString(info));
}

#if 0
ShaderReflectionDescriptor^ ShaderProgram::QueryReflectionDesc()
{
}
#endif

void ShaderProgram::BindConstantBuffer(String^ name, unsigned int bindingIndex)
{
    native_->BindConstantBuffer(ToStdString(name), bindingIndex);
}

void ShaderProgram::BindStorageBuffer(String^ name, unsigned int bindingIndex)
{
    native_->BindStorageBuffer(ToStdString(name), bindingIndex);
}


} // /namespace SharpLLGL

} // /namespace LHermanns



// ================================================================================
