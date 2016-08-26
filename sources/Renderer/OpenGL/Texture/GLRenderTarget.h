/*
 * GLRenderTarget.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_GL_RENDER_TARGET_H__
#define __LLGL_GL_RENDER_TARGET_H__


#include <LLGL/RenderTarget.h>
#include "GLFrameBuffer.h"
#include "GLRenderBuffer.h"
#include "GLTexture.h"


namespace LLGL
{


class GLRenderTarget : public RenderTarget
{

    public:

        void AttachDepthBuffer(const Gs::Vector2i& size) override;
        void AttachDepthStencilBuffer(const Gs::Vector2i& size) override;

        void AttachTexture1D(Texture& texture, int mipLevel = 0) override;
        void AttachTexture2D(Texture& texture, int mipLevel = 0) override;
        void AttachTexture3D(Texture& texture, int layer, int mipLevel = 0) override;
        void AttachTextureCube(Texture& texture, const AxisDirection cubeFace, int mipLevel = 0) override;
        void AttachTexture1DArray(Texture& texture, int layer, int mipLevel = 0) override;
        void AttachTexture2DArray(Texture& texture, int layer, int mipLevel = 0) override;
        void AttachTextureCubeArray(Texture& texture, int layer, const AxisDirection cubeFace, int mipLevel = 0) override;

        void DetachTextures() override;

        inline const GLFrameBuffer& GetFrameBuffer() const
        {
            return frameBuffer_;
        }

        inline const GLRenderBuffer& GetRenderBuffer() const
        {
            return renderBuffer_;
        }

    private:

        void ApplyMipResolution(Texture& texture, int mipLevel);

        void AttachRenderBuffer(const Gs::Vector2i& size, GLenum internalFormat, GLenum attachment);

        GLenum NextColorAttachment();

        GLFrameBuffer   frameBuffer_;
        GLRenderBuffer  renderBuffer_;

        int             attachments_            = 0;

        bool            renderBufferAttached_   = false;

};


} // /namespace LLGL


#endif



// ================================================================================
