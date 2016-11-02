/*
 * GLRenderbuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_RENDERBUFFER_H
#define LLGL_GL_RENDERBUFFER_H


#include "GLTexture.h"
#include <Gauss/Vector2.h>


namespace LLGL
{


class GLRenderbuffer
{

    public:

        GLRenderbuffer(const GLRenderbuffer&) = delete;
        GLRenderbuffer& operator = (const GLRenderbuffer&) = delete;

        GLRenderbuffer();
        ~GLRenderbuffer();

        void Bind() const;
        void Unbind() const;

        //! Recreates the internal renderbuffer object. This will invalidate the previous buffer ID.
        void Recreate();

        static void Storage(GLenum internalFormat, const Gs::Vector2i& size, GLsizei samples);

        //! Returns the hardware buffer ID.
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
