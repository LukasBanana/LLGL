/*
 * Shader.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/Shader.h>


namespace LLGL
{


Shader::Shader(const ShaderType type) :
    type_ { type }
{
}

Shader::~Shader()
{
}


} // /namespace LLGL



// ================================================================================
