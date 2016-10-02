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
    Buffer( type )
{
    glGenBuffers(1, &id_);
}

GLBuffer::~GLBuffer()
{
    glDeleteBuffers(1, &id_);
}

void GLBuffer::BufferData(const void* data, GLsizeiptr size, GLenum usage)
{
    glBufferData(GetTarget(), size, data, usage);
}

void GLBuffer::BufferSubData(const void* data, GLsizeiptr size, GLintptr offset)
{
    glBufferSubData(GetTarget(), offset, size, data);
}

void* GLBuffer::MapBuffer(GLenum access)
{
    return glMapBuffer(GetTarget(), access);
}

GLboolean GLBuffer::UnmapBuffer()
{
    return glUnmapBuffer(GetTarget());
}


/*
 * ======= Private: =======
 */

GLenum GLBuffer::GetTarget() const
{
    return GLTypes::Map(GetType());
}


} // /namespace LLGL



// ================================================================================
