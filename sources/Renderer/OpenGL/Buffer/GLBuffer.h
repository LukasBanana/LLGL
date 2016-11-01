/*
 * GLBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
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

        void BufferData(const void* data, GLsizeiptr size, GLenum usage);
        void BufferSubData(const void* data, GLsizeiptr size, GLintptr offset);

        void* MapBuffer(GLenum access);
        GLboolean UnmapBuffer();

        //! Returns the hardware buffer ID.
        inline GLuint GetID() const
        {
            return id_;
        }

    private:

        //! Returns the buffer target.
        GLenum GetTarget() const;

        GLuint id_ = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
