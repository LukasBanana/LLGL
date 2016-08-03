/*
 * GLVertexShader.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLVertexShader.h"


namespace LLGL
{


GLVertexShader::GLVertexShader() :
    hwShader(GL_VERTEX_SHADER)
{
}

bool GLVertexShader::Compile(const std::string& shaderSource)
{
    return hwShader.Compile(shaderSource);
}

std::string GLVertexShader::QueryInfoLog()
{
    return hwShader.QueryInfoLog();
}


} // /namespace LLGL



// ================================================================================
