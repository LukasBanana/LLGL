/*
 * GLRenderbuffer.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
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

        GLRenderbuffer() = default;
        ~GLRenderbuffer();

        GLRenderbuffer(const GLRenderbuffer&) = delete;
        GLRenderbuffer& operator = (const GLRenderbuffer&) = delete;

        GLRenderbuffer(GLRenderbuffer&& rhs);
        GLRenderbuffer& operator = (GLRenderbuffer&& rhs);

        void GenRenderbuffer();
        void DeleteRenderbuffer();

        // Binds the renderbuffer and initialized its storage.
        void BindAndAllocStorage(GLenum internalFormat, GLsizei width, GLsizei height, GLsizei samples);

        // Returns the hardware buffer ID.
        inline GLuint GetID() const
        {
            return id_;
        }

        // Returns true if this framebuffer object has a valid ID.
        inline bool Valid() const
        {
            return (GetID() != 0);
        }

        // Equivalent to Valid().
        inline operator bool () const
        {
            return Valid();
        }

    public:

        // Defines the storage for
        static void AllocStorage(GLuint id, GLenum internalFormat, GLsizei width, GLsizei height, GLsizei samples);

    private:

        GLuint id_ = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
