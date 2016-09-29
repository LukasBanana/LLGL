/*
 * GLBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_GL_BUFFER_H__
#define __LLGL_GL_BUFFER_H__


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

        //! Returns the buffer target.
        inline GLenum GetTarget() const
        {
            return target_;
        }

        //! Returns the hardware buffer ID.
        inline GLuint GetID() const
        {
            return id_;
        }

    private:

        GLenum target_  = 0;
        GLuint id_      = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
