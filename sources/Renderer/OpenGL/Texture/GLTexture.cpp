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

void GLTexture::Recreate()
{
    /* Delete previous texture and create a new one */
    glDeleteTextures(1, &id_);
    glGenTextures(1, &id_);
}


/*
 * ======= Protected: =======
 */

void GLTexture::SetType(const TextureType type)
{
    Texture::SetType(type);
}


} // /namespace LLGL



// ================================================================================
