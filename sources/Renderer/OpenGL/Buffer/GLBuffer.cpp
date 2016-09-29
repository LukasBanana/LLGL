/*
 * GLBuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLBuffer.h"
#include "../GLTypes.h"
#include "../Ext/GLExtensions.h"


namespace LLGL
{


GLBuffer::GLBuffer(const BufferType type) :
    Buffer  ( type               ),
    target_ ( GLTypes::Map(type) )
{
    glGenBuffers(1, &id_);
}

GLBuffer::~GLBuffer()
{
    glDeleteBuffers(1, &id_);
}

void GLBuffer::BufferData(const void* data, GLsizeiptr size, GLenum usage)
{
    glBufferData(target_, size, data, usage);
}

void GLBuffer::BufferSubData(const void* data, GLsizeiptr size, GLintptr offset)
{
    glBufferSubData(target_, offset, size, data);
}

void* GLBuffer::MapBuffer(GLenum access)
{
    return glMapBuffer(target_, access);
}

GLboolean GLBuffer::UnmapBuffer()
{
    return glUnmapBuffer(target_);
}


} // /namespace LLGL



// ================================================================================
