/*
 * GLFrameBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_GL_FRAME_BUFFER_H__
#define __LLGL_GL_FRAME_BUFFER_H__


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

        GLFrameBuffer(GLenum target);
        ~GLFrameBuffer();

        void Bind() const;
        void Unbind() const;

        //! Recreates the internal framebuffer object. This will invalidate the previous buffer ID.
        void Recreate();

        void AttachTexture1D(GLenum attachment, GLTexture& texture, GLenum textureTarget, GLint mipLevel);
        void AttachTexture2D(GLenum attachment, GLTexture& texture, GLenum textureTarget, GLint mipLevel);
        void AttachTexture3D(GLenum attachment, GLTexture& texture, GLenum textureTarget, GLint mipLevel, GLint zOffset);
        void AttachTextureLayer(GLenum attachment, GLTexture& texture, GLint mipLevel, GLint layer);
        
        void AttachRenderBuffer(GLenum attachment, GLRenderBuffer& renderBuffer);

        void Blit(const Gs::Vector2i& size, GLenum mask, GLenum filter);
        
        void Blit(
            const Gs::Vector2i& srcPos0, const Gs::Vector2i& srcPos1,
            const Gs::Vector2i& destPos0, const Gs::Vector2i& destPos1,
            GLenum mask, GLenum filter
        );

        GLenum CheckStatus() const;

        //! Returns the framebuffer target.
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

        GLenum target_  = GL_DRAW_FRAMEBUFFER;
        GLuint id_      = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
