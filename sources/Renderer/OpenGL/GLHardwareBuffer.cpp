/*
 * GLHardwareBuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLHardwareBuffer.h"
#include "GLExtensions.h"


namespace LLGL
{


GLHardwareBuffer::GLHardwareBuffer(GLenum target) :
    target_( target )
{
    glGenBuffers(1, &id_);
}

GLHardwareBuffer::~GLHardwareBuffer()
{
    glDeleteBuffers(1, &id_);
}


} // /namespace LLGL



// ================================================================================
