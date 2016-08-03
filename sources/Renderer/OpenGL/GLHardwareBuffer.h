/*
 * GLHardwareBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_GL_HARDWARE_BUFFER_H__
#define __LLGL_GL_HARDWARE_BUFFER_H__


#include "OpenGL.h"


namespace LLGL
{


class GLHardwareBuffer
{

    public:

        GLHardwareBuffer(const GLHardwareBuffer&) = delete;
        GLHardwareBuffer& operator = (const GLHardwareBuffer&) = delete;

        GLHardwareBuffer(GLenum target);
        ~GLHardwareBuffer();

        void BufferData(const void* data, const GLsizeiptr size, const GLenum usage);
        void BufferSubData(const void* data, const GLsizeiptr size, const GLintptr offset);

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
