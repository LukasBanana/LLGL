/*
 * GLComputeShader.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLComputeShader.h"


namespace LLGL
{


GLComputeShader::GLComputeShader() :
    hwShader(GL_COMPUTE_SHADER)
{
}

bool GLComputeShader::Compile(const std::string& shaderSource)
{
    return hwShader.Compile(shaderSource);
}

std::string GLComputeShader::QueryInfoLog()
{
    return hwShader.QueryInfoLog();
}


} // /namespace LLGL



// ================================================================================
