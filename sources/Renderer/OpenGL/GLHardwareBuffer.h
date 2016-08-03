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

        //! Returns the hardware buffer ID.
        inline GLuint Get() const
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
