/*
 * GLFramebuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_FRAMEBUFFER_H
#define LLGL_GL_FRAMEBUFFER_H


#include "../RenderState/GLState.h"
#include <Gauss/Vector2.h>


namespace LLGL
{


class GLFramebuffer
{

    public:

        GLFramebuffer(const GLFramebuffer&) = delete;
        GLFramebuffer& operator = (const GLFramebuffer&) = delete;

        GLFramebuffer();
        ~GLFramebuffer();

        void Bind(const GLFramebufferTarget target = GLFramebufferTarget::FRAMEBUFFER) const;
        void Unbind(const GLFramebufferTarget target = GLFramebufferTarget::FRAMEBUFFER) const;

        // Recreates the internal framebuffer object. This will invalidate the previous buffer ID.
        void Recreate();

        static void AttachTexture1D(GLenum attachment, GLenum textureTarget, GLuint textureID, GLint mipLevel);
        static void AttachTexture2D(GLenum attachment, GLenum textureTarget, GLuint textureID, GLint mipLevel);
        static void AttachTexture3D(GLenum attachment, GLenum textureTarget, GLuint textureID, GLint mipLevel, GLint zOffset);
        static void AttachTextureLayer(GLenum attachment, GLuint textureID, GLint mipLevel, GLint layer);
        
        static void AttachRenderbuffer(GLenum attachment, GLuint renderbufferID);

        static void Blit(const Gs::Vector2i& size, GLenum mask);
        
        static void Blit(
            const Gs::Vector2i& srcPos0, const Gs::Vector2i& srcPos1,
            const Gs::Vector2i& destPos0, const Gs::Vector2i& destPos1,
            GLenum mask, GLenum filter
        );

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
