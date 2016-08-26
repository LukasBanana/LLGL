/*
 * GLFrameBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_GL_FRAME_BUFFER_H__
#define __LLGL_GL_FRAME_BUFFER_H__


#include "GLTexture.h"


namespace LLGL
{


class GLFrameBuffer
{

    public:

        GLFrameBuffer(const GLFrameBuffer&) = delete;
        GLFrameBuffer& operator = (const GLFrameBuffer&) = delete;

        GLFrameBuffer();
        ~GLFrameBuffer();

        void Bind() const;
        void Unbind() const;

        //! Recreates the internal framebuffer object. This will invalidate the previous buffer ID.
        void Recreate();

        void AttachTexture1D(GLenum attachment, GLTexture& texture, GLenum textureTarget, GLint mipLevel);
        void AttachTexture2D(GLenum attachment, GLTexture& texture, GLenum textureTarget, GLint mipLevel);
        void AttachTexture3D(GLenum attachment, GLTexture& texture, GLenum textureTarget, GLint mipLevel, GLint zOffset);
        void AttachTextureLayer(GLenum attachment, GLTexture& texture, GLint mipLevel, GLint layer);

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
