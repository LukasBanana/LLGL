/*
 * GLGeometryShader.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLGeometryShader.h"


namespace LLGL
{


GLGeometryShader::GLGeometryShader() :
    hwShader(GL_GEOMETRY_SHADER)
{
}

bool GLGeometryShader::Compile(const std::string& shaderSource)
{
    return hwShader.Compile(shaderSource);
}

std::string GLGeometryShader::QueryInfoLog()
{
    return hwShader.QueryInfoLog();
}


} // /namespace LLGL



// ================================================================================
