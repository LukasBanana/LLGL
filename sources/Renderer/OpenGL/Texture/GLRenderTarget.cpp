/*
 * GLRenderTarget.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLRenderTarget.h"
#include "../RenderState/GLStateManager.h"
#include "../GLExtensions.h"
#include "../../CheckedCast.h"
#include "../GLTypes.h"
#include "../GLCore.h"
#include "../../../Core/Helper.h"


namespace LLGL
{


GLRenderTarget::GLRenderTarget(unsigned int multiSamples) :
    multiSamples_( static_cast<GLsizei>(multiSamples) )
{
    if (multiSamples_ > 0)
        frameBufferMS_ = MakeUnique<GLFrameBuffer>();
}

void GLRenderTarget::AttachDepthBuffer(const Gs::Vector2i& size)
{
    AttachRenderBuffer(size, GL_DEPTH_COMPONENT, GL_DEPTH_ATTACHMENT);
    blitMask_ |= GL_DEPTH_BUFFER_BIT;
}

void GLRenderTarget::AttachStencilBuffer(const Gs::Vector2i& size)
{
    AttachRenderBuffer(size, GL_STENCIL_INDEX, GL_STENCIL_ATTACHMENT);
    blitMask_ |= GL_STENCIL_BUFFER_BIT;
}

void GLRenderTarget::AttachDepthStencilBuffer(const Gs::Vector2i& size)
{
    AttachRenderBuffer(size, GL_DEPTH_STENCIL, GL_DEPTH_STENCIL_ATTACHMENT);
    blitMask_ |= (GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void GLRenderTarget::AttachTexture1D(Texture& texture, int mipLevel)
{
    AttachTexture(
        texture, mipLevel,
        [&](GLenum attachment, GLTexture& textureGL)
        {
            GLFrameBuffer::AttachTexture1D(attachment, textureGL, GL_TEXTURE_1D, mipLevel);
        }
    );
}

void GLRenderTarget::AttachTexture2D(Texture& texture, int mipLevel)
{
    AttachTexture(
        texture, mipLevel,
        [&](GLenum attachment, GLTexture& textureGL)
        {
            GLFrameBuffer::AttachTexture2D(attachment, textureGL, GL_TEXTURE_2D, mipLevel);
        }
    );
}

void GLRenderTarget::AttachTexture3D(Texture& texture, int layer, int mipLevel)
{
    AttachTexture(
        texture, mipLevel,
        [&](GLenum attachment, GLTexture& textureGL)
        {
            GLFrameBuffer::AttachTexture3D(attachment, textureGL, GL_TEXTURE_3D, mipLevel, layer);
        }
    );
}

void GLRenderTarget::AttachTextureCube(Texture& texture, const AxisDirection cubeFace, int mipLevel)
{
    AttachTexture(
        texture, mipLevel,
        [&](GLenum attachment, GLTexture& textureGL)
        {
            GLFrameBuffer::AttachTexture2D(attachment, textureGL, GLTypes::Map(cubeFace), mipLevel);
        }
    );
}

void GLRenderTarget::AttachTexture1DArray(Texture& texture, int layer, int mipLevel)
{
    AttachTexture(
        texture, mipLevel,
        [&](GLenum attachment, GLTexture& textureGL)
        {
            GLFrameBuffer::AttachTextureLayer(attachment, textureGL, mipLevel, layer);
        }
    );
}

void GLRenderTarget::AttachTexture2DArray(Texture& texture, int layer, int mipLevel)
{
    AttachTexture(
        texture, mipLevel,
        [&](GLenum attachment, GLTexture& textureGL)
        {
            GLFrameBuffer::AttachTextureLayer(attachment, textureGL, mipLevel, layer);
        }
    );
}

void GLRenderTarget::AttachTextureCubeArray(Texture& texture, int layer, const AxisDirection cubeFace, int mipLevel)
{
    AttachTexture(
        texture, mipLevel,
        [&](GLenum attachment, GLTexture& textureGL)
        {
            GLFrameBuffer::AttachTextureLayer(attachment, textureGL, mipLevel, layer * 6 + static_cast<int>(cubeFace));
        }
    );
}

void GLRenderTarget::DetachTextures()
{
    /* Recreate frame buffer, and reset resolution and other parameters (except multi-sample parameter) */
    frameBuffer_.Recreate();

    renderBuffer_.reset();

    if (frameBufferMS_)
        frameBufferMS_->Recreate();

    ResetResolution();

    colorAttachments_.clear();
    renderBuffersMS_.clear();

    blitMask_ = 0;
}

/* ----- Extended Internal Functions ----- */

void GLRenderTarget::BlitOntoFrameBuffer()
{
    if (frameBufferMS_)
    {
        frameBuffer_.Bind(GLFrameBufferTarget::DRAW_FRAMEBUFFER);
        frameBufferMS_->Bind(GLFrameBufferTarget::READ_FRAMEBUFFER);

        for (auto attachment : colorAttachments_)
        {
            /*
            Blit (or rather copy) each attachment from the
            multi-sample frame buffer into the main frame buffer
            */
            glReadBuffer(attachment);
            glDrawBuffer(attachment);

            GLFrameBuffer::Blit(GetResolution(), blitMask_);
        }

        frameBufferMS_->Unbind(GLFrameBufferTarget::READ_FRAMEBUFFER);
        frameBuffer_.Unbind(GLFrameBufferTarget::DRAW_FRAMEBUFFER);
    }
}

void GLRenderTarget::BlitOntoScreen(std::size_t colorAttachmentIndex)
{
    if (colorAttachmentIndex < colorAttachments_.size())
    {
        const auto& frameBuffer = GetFrameBuffer();

        GLStateManager::active->BindFrameBuffer(GLFrameBufferTarget::DRAW_FRAMEBUFFER, 0);
        GLStateManager::active->BindFrameBuffer(GLFrameBufferTarget::READ_FRAMEBUFFER, frameBuffer.GetID());
        {
            glReadBuffer(colorAttachments_[colorAttachmentIndex]);
            glDrawBuffer(GL_BACK);

            GLFrameBuffer::Blit(GetResolution(), blitMask_);
        }
        GLStateManager::active->BindFrameBuffer(GLFrameBufferTarget::READ_FRAMEBUFFER, 0);
    }
}

const GLFrameBuffer& GLRenderTarget::GetFrameBuffer() const
{
    return (frameBufferMS_ != nullptr ? *frameBufferMS_ : frameBuffer_);
}


/*
 * ======= Private: =======
 */

static GLenum CheckDrawFramebufferStatus()
{
    return glCheckFramebufferStatus(GL_FRAMEBUFFER);
}

void GLRenderTarget::ApplyMipResolution(Texture& texture, int mipLevel)
{
    /* Apply texture size to frame buffer resolution */
    auto size = texture.QueryMipLevelSize(mipLevel);
    ApplyResolution({ size.x, size.y });
}

void GLRenderTarget::InitRenderBufferStorage(GLRenderBuffer& renderBuffer, GLenum internalFormat)
{
    renderBuffer.Bind();
    {
        GLRenderBuffer::Storage(internalFormat, GetResolution(), multiSamples_);
    }
    renderBuffer.Unbind();
}

GLenum GLRenderTarget::AttachDefaultRenderBuffer(GLFrameBuffer& frameBuffer, GLenum attachment)
{
    GLenum status = 0;

    frameBuffer.Bind();
    {
        GLFrameBuffer::AttachRenderBuffer(attachment, *renderBuffer_);
        status = CheckDrawFramebufferStatus();
    }
    frameBuffer.Unbind();

    return status;
}

void GLRenderTarget::AttachRenderBuffer(const Gs::Vector2i& size, GLenum internalFormat, GLenum attachment)
{
    if (!renderBuffer_)
    {
        renderBuffer_ = MakeUnique<GLRenderBuffer>();

        /* Apply size to frame buffer resolution */
        ApplyResolution(size);

        /* Setup render buffer storage */
        InitRenderBufferStorage(*renderBuffer_, internalFormat);

        /* Attach render buffer to frame buffer (or multi-sample frame buffer if multi-sampling is used) */
        GLenum status = 0;

        if (frameBufferMS_)
        {
            status = AttachDefaultRenderBuffer(*frameBufferMS_, attachment);
            CheckFrameBufferStatus(status, "depth- or depth-stencil attachment to multi-sample frame buffer object (FBO)");
        }
        else
        {
            status = AttachDefaultRenderBuffer(frameBuffer_, attachment);
            CheckFrameBufferStatus(status, "depth- or depth-stencil attachment to frame buffer object (FBO)");
        }
    }
    else
        throw std::runtime_error("attachment to render target failed, because render target already has a depth- or depth-stencil buffer");
}

static GLenum GetTexInternalFormat(const GLTexture& textureGL)
{
    GLint internalFormat = GL_RGBA;
    {
        GLStateManager::active->BindTexture(textureGL);
        glGetTexLevelParameteriv(GLTypes::Map(textureGL.GetType()), 0, GL_TEXTURE_INTERNAL_FORMAT, &internalFormat);
    }
    return internalFormat;
}

void GLRenderTarget::AttachTexture(Texture& texture, int mipLevel, const AttachTextureCallback& attachmentProc)
{
    /* Get OpenGL texture object */
    auto& textureGL = LLGL_CAST(GLTexture&, texture);

    /* Apply resolution for MIP-map level */
    ApplyMipResolution(texture, mipLevel);

    GLenum status = 0;
    GLenum attachment = MakeColorAttachment();

    /* Attach texture to frame buffer (via callback) */
    frameBuffer_.Bind();
    {
        attachmentProc(attachment, textureGL);
        status = CheckDrawFramebufferStatus();
        
        /* Set draw buffers for this frame buffer is multi-sampling is disabled */
        if (!frameBufferMS_)
            SetDrawBuffers();
    }
    frameBuffer_.Unbind();

    CheckFrameBufferStatus(status, "color attachment to frame buffer object (FBO)");

    /* Create render buffer for attachment if multi-sample frame buffer is used */
    if (frameBufferMS_)
    {
        auto renderBuffer = MakeUnique<GLRenderBuffer>();
        {
            /* Setup render buffer storage by texture's internal format */
            InitRenderBufferStorage(*renderBuffer, GetTexInternalFormat(textureGL));

            /* Attach render buffer to multi-sample frame buffer */
            frameBufferMS_->Bind();
            {
                GLFrameBuffer::AttachRenderBuffer(attachment, *renderBuffer);
                status = CheckDrawFramebufferStatus();
                
                /* Set draw buffers for this frame buffer is multi-sampling is enabled */
                SetDrawBuffers();
            }
            frameBufferMS_->Unbind();

            CheckFrameBufferStatus(status, "color attachment to multi-sample frame buffer object (FBO)");
        }
        renderBuffersMS_.emplace_back(std::move(renderBuffer));
    }

    /* Add color buffer bit to blit mask */
    blitMask_ |= GL_COLOR_BUFFER_BIT;
}

GLenum GLRenderTarget::MakeColorAttachment()
{
    GLenum attachment = (GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(colorAttachments_.size()));
    colorAttachments_.push_back(attachment);
    return attachment;
}

void GLRenderTarget::SetDrawBuffers()
{
    /*
    Tell OpenGL which buffers are to be written when drawing operations are performed.
    Each color attachment has its own draw buffer.
    */
    if (colorAttachments_.empty())
        glDrawBuffer(GL_NONE);
    else if (colorAttachments_.size() == 1)
        glDrawBuffer(colorAttachments_.front());
    else
        glDrawBuffers(static_cast<GLsizei>(colorAttachments_.size()), colorAttachments_.data());
}

void GLRenderTarget::CheckFrameBufferStatus(GLenum status, const std::string& info)
{
    if (status != GL_FRAMEBUFFER_COMPLETE)
        throw std::runtime_error(info + " failed (error code = " + GLErrorToStr(status) + ")");
}


} // /namespace LLGL



// ================================================================================
