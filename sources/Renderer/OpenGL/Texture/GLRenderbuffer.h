/*
 * GLRenderbuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_RENDERBUFFER_H
#define LLGL_GL_RENDERBUFFER_H


#include "../OpenGL.h"


namespace LLGL
{


// Wrapper class for GL renderbuffer objects (RBOs).
class GLRenderbuffer
{

    public:

        GLRenderbuffer(const GLRenderbuffer&) = delete;
        GLRenderbuffer& operator = (const GLRenderbuffer&) = delete;

        GLRenderbuffer();
        ~GLRenderbuffer();

        // Binds the renderbuffer and initialized its storage.
        void Storage(GLenum internalFormat, GLsizei width, GLsizei height, GLsizei samples);

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
