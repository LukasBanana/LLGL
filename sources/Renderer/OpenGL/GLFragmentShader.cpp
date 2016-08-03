/*
 * GLFragmentShader.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLFragmentShader.h"


namespace LLGL
{


GLFragmentShader::GLFragmentShader() :
    hwShader(GL_FRAGMENT_SHADER)
{
}

bool GLFragmentShader::Compile(const std::string& shaderSource)
{
    return hwShader.Compile(shaderSource);
}

std::string GLFragmentShader::QueryInfoLog()
{
    return hwShader.QueryInfoLog();
}


} // /namespace LLGL



// ================================================================================
