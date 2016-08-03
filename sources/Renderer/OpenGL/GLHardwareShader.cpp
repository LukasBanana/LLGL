/*
 * GLHardwareShader.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLHardwareShader.h"
#include "GLExtensions.h"


namespace LLGL
{


GLHardwareShader::GLHardwareShader(GLenum shaderType)
{
    id_ = glCreateShader(shaderType);
}

GLHardwareShader::~GLHardwareShader()
{
    glDeleteShader(id_);
}


} // /namespace LLGL



// ================================================================================
