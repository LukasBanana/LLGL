/*
 * GLFrameBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_FRAME_BUFFER_H
#define LLGL_GL_FRAME_BUFFER_H


#include "GLTexture.h"
#include "../RenderState/GLState.h"
#include <Gauss/Vector2.h>


namespace LLGL
{


class GLRenderBuffer;

class GLFrameBuffer
{

    public:

        GLFrameBuffer(const GLFrameBuffer&) = delete;
        GLFrameBuffer& operator = (const GLFrameBuffer&) = delete;

        GLFrameBuffer();
        ~GLFrameBuffer();

        void Bind(const GLFrameBufferTarget target = GLFrameBufferTarget::FRAMEBUFFER) const;
        void Unbind(const GLFrameBufferTarget target = GLFrameBufferTarget::FRAMEBUFFER) const;

        //! Recreates the internal framebuffer object. This will invalidate the previous buffer ID.
        void Recreate();

        static void AttachTexture1D(GLenum attachment, GLTexture& texture, GLenum textureTarget, GLint mipLevel);
        static void AttachTexture2D(GLenum attachment, GLTexture& texture, GLenum textureTarget, GLint mipLevel);
        static void AttachTexture3D(GLenum attachment, GLTexture& texture, GLenum textureTarget, GLint mipLevel, GLint zOffset);
        static void AttachTextureLayer(GLenum attachment, GLTexture& texture, GLint mipLevel, GLint layer);
        
        static void AttachRenderBuffer(GLenum attachment, GLRenderBuffer& renderBuffer);

        static void Blit(const Gs::Vector2i& size, GLenum mask);
        
        static void Blit(
            const Gs::Vector2i& srcPos0, const Gs::Vector2i& srcPos1,
            const Gs::Vector2i& destPos0, const Gs::Vector2i& destPos1,
            GLenum mask, GLenum filter
        );

        //! Returns the hardware buffer ID.
        inline GLuint GetID() const
        {
            return id_;
        }

    private:

        GLuint id_      = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
