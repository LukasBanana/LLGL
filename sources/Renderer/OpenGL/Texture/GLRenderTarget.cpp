/*
 * GLRenderTarget.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLRenderTarget.h"
#include "../../CheckedCast.h"
#include "../GLTypes.h"


namespace LLGL
{


void GLRenderTarget::AttachDepthBuffer(const Gs::Vector2i& size)
{
    AttachRenderBuffer(size, GL_DEPTH_COMPONENT, GL_DEPTH_ATTACHMENT);
}

void GLRenderTarget::AttachDepthStencilBuffer(const Gs::Vector2i& size)
{
    AttachRenderBuffer(size, GL_DEPTH_STENCIL, GL_DEPTH_STENCIL_ATTACHMENT);
}

void GLRenderTarget::AttachTexture1D(Texture& texture, int mipLevel)
{
    ApplyMipResolution(texture, mipLevel);

    auto& textureGL = LLGL_CAST(GLTexture&, texture);
    frameBuffer_.Bind(GLFrameBufferTarget::DRAW_FRAMEBUFFER);
    {
        frameBuffer_.AttachTexture1D(NextColorAttachment(), textureGL, GL_TEXTURE_1D, mipLevel);
    }
    frameBuffer_.Unbind(GLFrameBufferTarget::DRAW_FRAMEBUFFER);
}

void GLRenderTarget::AttachTexture2D(Texture& texture, int mipLevel)
{
    ApplyMipResolution(texture, mipLevel);

    auto& textureGL = LLGL_CAST(GLTexture&, texture);
    frameBuffer_.Bind(GLFrameBufferTarget::DRAW_FRAMEBUFFER);
    {
        frameBuffer_.AttachTexture2D(NextColorAttachment(), textureGL, GL_TEXTURE_2D, mipLevel);
    }
    frameBuffer_.Unbind(GLFrameBufferTarget::DRAW_FRAMEBUFFER);
}

void GLRenderTarget::AttachTexture3D(Texture& texture, int layer, int mipLevel)
{
    ApplyMipResolution(texture, mipLevel);

    auto& textureGL = LLGL_CAST(GLTexture&, texture);
    frameBuffer_.Bind(GLFrameBufferTarget::DRAW_FRAMEBUFFER);
    {
        frameBuffer_.AttachTexture3D(NextColorAttachment(), textureGL, GL_TEXTURE_3D, mipLevel, layer);
    }
    frameBuffer_.Unbind(GLFrameBufferTarget::DRAW_FRAMEBUFFER);
}

void GLRenderTarget::AttachTextureCube(Texture& texture, const AxisDirection cubeFace, int mipLevel)
{
    ApplyMipResolution(texture, mipLevel);

    auto& textureGL = LLGL_CAST(GLTexture&, texture);
    frameBuffer_.Bind(GLFrameBufferTarget::DRAW_FRAMEBUFFER);
    {
        frameBuffer_.AttachTexture2D(NextColorAttachment(), textureGL, GLTypes::Map(cubeFace), mipLevel);
    }
    frameBuffer_.Unbind(GLFrameBufferTarget::DRAW_FRAMEBUFFER);
}

void GLRenderTarget::AttachTexture1DArray(Texture& texture, int layer, int mipLevel)
{
    ApplyMipResolution(texture, mipLevel);

    auto& textureGL = LLGL_CAST(GLTexture&, texture);
    frameBuffer_.Bind(GLFrameBufferTarget::DRAW_FRAMEBUFFER);
    {
        frameBuffer_.AttachTextureLayer(NextColorAttachment(), textureGL, mipLevel, layer);
    }
    frameBuffer_.Unbind(GLFrameBufferTarget::DRAW_FRAMEBUFFER);
}

void GLRenderTarget::AttachTexture2DArray(Texture& texture, int layer, int mipLevel)
{
    ApplyMipResolution(texture, mipLevel);

    auto& textureGL = LLGL_CAST(GLTexture&, texture);
    frameBuffer_.Bind(GLFrameBufferTarget::DRAW_FRAMEBUFFER);
    {
        frameBuffer_.AttachTextureLayer(NextColorAttachment(), textureGL, mipLevel, layer);
    }
    frameBuffer_.Unbind(GLFrameBufferTarget::DRAW_FRAMEBUFFER);
}

void GLRenderTarget::AttachTextureCubeArray(Texture& texture, int layer, const AxisDirection cubeFace, int mipLevel)
{
    ApplyMipResolution(texture, mipLevel);

    auto& textureGL = LLGL_CAST(GLTexture&, texture);
    frameBuffer_.Bind(GLFrameBufferTarget::DRAW_FRAMEBUFFER);
    {
        frameBuffer_.AttachTextureLayer(NextColorAttachment(), textureGL, mipLevel, layer * 6 + static_cast<int>(cubeFace));
    }
    frameBuffer_.Unbind(GLFrameBufferTarget::DRAW_FRAMEBUFFER);
}

void GLRenderTarget::DetachTextures()
{
    /*
    Just recreate framebuffer and renderbuffer,
    since some graphics drivers have problems to resize a framebuffer
    */
    frameBuffer_.Recreate();
    renderBuffer_.Recreate();

    /* Reset resolution and other parameters */
    ResetResolution();

    attachments_            = 0;
    renderBufferAttached_   = false;
}


/*
 * ======= Private: =======
 */

void GLRenderTarget::ApplyMipResolution(Texture& texture, int mipLevel)
{
    /* Apply texture size to frame buffer resolution */
    auto size = texture.QueryMipLevelSize(mipLevel);
    ApplyResolution({ size.x, size.y });
}

void GLRenderTarget::AttachRenderBuffer(const Gs::Vector2i& size, GLenum internalFormat, GLenum attachment)
{
    if (!renderBufferAttached_)
    {
        /* Apply size to frame buffer resolution */
        ApplyResolution(size);

        /* Setup render buffer storage */
        renderBuffer_.Storage(internalFormat, GetResolution(), 0);

        frameBuffer_.Bind(GLFrameBufferTarget::DRAW_FRAMEBUFFER);
        {
            frameBuffer_.AttachRenderBuffer(attachment, renderBuffer_);
        }
        frameBuffer_.Unbind(GLFrameBufferTarget::DRAW_FRAMEBUFFER);

        renderBufferAttached_ = true;
    }
    else
        throw std::runtime_error("attachment to render target failed, because render target already has a depth- or depth-stencil buffer");
}

GLenum GLRenderTarget::NextColorAttachment()
{
    return (GL_COLOR_ATTACHMENT0 + (attachments_++));
}


} // /namespace LLGL



// ================================================================================
