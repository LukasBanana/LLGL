/*
 * GLTessEvaluationShader.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLTessEvaluationShader.h"


namespace LLGL
{


GLTessEvaluationShader::GLTessEvaluationShader() :
    hwShader(GL_TESS_EVALUATION_SHADER)
{
}

bool GLTessEvaluationShader::Compile(const std::string& shaderSource)
{
    return hwShader.Compile(shaderSource);
}

std::string GLTessEvaluationShader::QueryInfoLog()
{
    return hwShader.QueryInfoLog();
}


} // /namespace LLGL



// ================================================================================
