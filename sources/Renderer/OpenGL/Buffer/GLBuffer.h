/*
 * GLBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_BUFFER_H
#define LLGL_GL_BUFFER_H


#include <LLGL/Buffer.h>
#include "../OpenGL.h"


namespace LLGL
{


class GLBuffer : public Buffer
{

    public:

        GLBuffer(const BufferType type);
        ~GLBuffer();

        void BufferStorage(GLsizeiptr size, const void* data, GLbitfield flags, GLenum usage);
        void BufferSubData(GLintptr offset, GLsizeiptr size, const void* data);
        void CopyBufferSubData(const GLBuffer& readBuffer, GLintptr readOffset, GLintptr writeOffset, GLsizeiptr size);
        void* MapBuffer(GLenum access);
        void UnmapBuffer();

        // Returns the hardware buffer ID.
        inline GLuint GetID() const
        {
            return id_;
        }

    private:

        GLuint id_ = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
