/*
 * GLFramebuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_FRAMEBUFFER_H
#define LLGL_GL_FRAMEBUFFER_H


#include "../RenderState/GLState.h"
#include <LLGL/Types.h>


namespace LLGL
{


// Wrapper class for GL framebuffer objects (FBOs).
class GLFramebuffer
{

    public:

        GLFramebuffer(const GLFramebuffer&) = delete;
        GLFramebuffer& operator = (const GLFramebuffer&) = delete;

        GLFramebuffer() = default;
        ~GLFramebuffer();

        void GenFramebuffer();
        void DeleteFramebuffer();

        void Bind(const GLFramebufferTarget target = GLFramebufferTarget::FRAMEBUFFER) const;
        void Unbind(const GLFramebufferTarget target = GLFramebufferTarget::FRAMEBUFFER) const;

        static void AttachTexture1D(GLenum attachment, GLenum textureTarget, GLuint textureID, GLint mipLevel);
        static void AttachTexture2D(GLenum attachment, GLenum textureTarget, GLuint textureID, GLint mipLevel);
        static void AttachTexture3D(GLenum attachment, GLenum textureTarget, GLuint textureID, GLint mipLevel, GLint zOffset);
        static void AttachTextureLayer(GLenum attachment, GLuint textureID, GLint mipLevel, GLint layer);

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

    private:

        GLuint id_ = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
