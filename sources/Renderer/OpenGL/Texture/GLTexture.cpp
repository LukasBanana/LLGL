/*
 * GLTexture.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLTexture.h"


namespace LLGL
{


GLTexture::GLTexture()
{
    glGenTextures(1, &id_);
}

GLTexture::~GLTexture()
{
    glDeleteTextures(1, &id_);
}


} // /namespace LLGL



// ================================================================================
