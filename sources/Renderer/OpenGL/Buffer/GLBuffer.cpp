/*
 * GLBuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLBuffer.h"
#include "../../GLCommon/GLTypes.h"
#include "../Ext/GLExtensions.h"


namespace LLGL
{


GLBuffer::GLBuffer(const BufferType type) :
    Buffer { type }
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
    #ifdef LLGL_GL_OPENGLES
    //TODO: move this into "Renderer/OpenGLES2/Buffer/GLES2Buffer.cpp"
    return glMapBufferOES(GetTarget(), access);
    #else
    return glMapBuffer(GetTarget(), access);
    #endif
}

GLboolean GLBuffer::UnmapBuffer()
{
    #ifdef LLGL_GL_OPENGLES
    //TODO: move this into "Renderer/OpenGLES2/Buffer/GLES2Buffer.cpp"
    return glUnmapBufferOES(GetTarget());
    #else
    return glUnmapBuffer(GetTarget());
    #endif
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
