/*
 * GLHardwareBuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLHardwareBuffer.h"
#include "../Ext/GLExtensions.h"


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

void GLHardwareBuffer::BufferData(const void* data, GLsizeiptr size, GLenum usage)
{
    glBufferData(target_, size, data, usage);
}

void GLHardwareBuffer::BufferSubData(const void* data, GLsizeiptr size, GLintptr offset)
{
    glBufferSubData(target_, offset, size, data);
}

void* GLHardwareBuffer::MapBuffer(GLenum access)
{
    return glMapBuffer(target_, access);
}

GLboolean GLHardwareBuffer::UnmapBuffer()
{
    return glUnmapBuffer(target_);
}


} // /namespace LLGL



// ================================================================================
