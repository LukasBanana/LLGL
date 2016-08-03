/*
 * GLTessControlShader.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLTessControlShader.h"


namespace LLGL
{


GLTessControlShader::GLTessControlShader() :
    hwShader(GL_TESS_CONTROL_SHADER)
{
}

bool GLTessControlShader::Compile(const std::string& shaderSource)
{
    return hwShader.Compile(shaderSource);
}

std::string GLTessControlShader::QueryInfoLog()
{
    return hwShader.QueryInfoLog();
}


} // /namespace LLGL



// ================================================================================
