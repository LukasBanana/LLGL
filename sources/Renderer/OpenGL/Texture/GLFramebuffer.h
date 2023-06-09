/*
 * GLFramebuffer.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_GL_FRAMEBUFFER_H
#define LLGL_GL_FRAMEBUFFER_H


#include "../RenderState/GLState.h"
#include <LLGL/Types.h>


namespace LLGL
{


class GLTexture;

// Helper container for a pair of OpenGL FBOs.
struct GLFramebufferPair
{
    ~GLFramebufferPair();

    void CreateFBOs();
    void ReleaseFBOs();

    GLuint fbos[2] = { 0, 0 };
};

// Wrapper class for GL framebuffer objects (FBOs).
class GLFramebuffer
{

    public:

        GLFramebuffer() = default;
        ~GLFramebuffer();

        GLFramebuffer(const GLFramebuffer&) = delete;
        GLFramebuffer& operator = (const GLFramebuffer&) = delete;

        GLFramebuffer(GLFramebuffer&& rhs);
        GLFramebuffer& operator = (GLFramebuffer&& rhs);

        void GenFramebuffer();
        void DeleteFramebuffer();

        // Sets the default framebuffer parameters and return true on success, otherwise the "GL_ARB_framebuffer_no_attachments" extension is not supported.
        bool FramebufferParameters(
            GLint width,
            GLint height,
            GLint layers,
            GLint samples,
            GLint fixedSampleLocations
        );

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

        static void AttachTexture(
            const GLTexture&    texture,
            GLenum              attachment,
            GLint               mipLevel,
            GLint               arrayLayer,
            GLenum              target = GL_FRAMEBUFFER
        );

        static void AttachRenderbuffer(GLenum attachment, GLuint renderbufferID);

        static void Blit(GLint width, GLint height, GLenum mask);

        static void Blit(
            const Offset2D& srcPos0,
            const Offset2D& srcPos1,
            const Offset2D& destPos0,
            const Offset2D& destPos1,
            GLenum          mask,
            GLenum          filter
        );

    private:

        GLuint id_ = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
