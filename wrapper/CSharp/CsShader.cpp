/*
 * CsShader.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "CsShader.h"
#include "CsHelper.h"


namespace SharpLLGL
{


Shader::Shader(LLGL::Shader* native) :
    native_ { native }
{
}

LLGL::Shader* Shader::Native::get()
{
    return native_;
}

bool Shader::HasErrors::get()
{
    return native_->HasErrors();
}

String^ Shader::Report::get()
{
    return gcnew String(ToManagedString(native_->GetReport()));
}

ShaderType Shader::Type::get()
{
    return static_cast<ShaderType>(native_->GetType());
}

SharpLLGL::StageFlags Shader::StageFlags::get()
{
    return static_cast<SharpLLGL::StageFlags>(native_->GetStageFlags());
};


} // /namespace SharpLLGL



// ================================================================================
