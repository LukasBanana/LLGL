/*
 * GLRenderTarget.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLRenderTarget.h"
#include "../RenderState/GLStateManager.h"
#include "../Ext/GLExtensions.h"
#include "../../CheckedCast.h"
#include "../GLTypes.h"
#include "../GLCore.h"
#include "../../../Core/Helper.h"


namespace LLGL
{


GLRenderTarget::GLRenderTarget(unsigned int multiSamples) :
    multiSamples_( static_cast<GLsizei>(multiSamples) )
{
}

void GLRenderTarget::AttachDepthBuffer(const Gs::Vector2ui& size)
{
    AttachRenderBuffer(size, GL_DEPTH_COMPONENT, GL_DEPTH_ATTACHMENT);
    blitMask_ |= GL_DEPTH_BUFFER_BIT;
}

void GLRenderTarget::AttachStencilBuffer(const Gs::Vector2ui& size)
{
    AttachRenderBuffer(size, GL_STENCIL_INDEX, GL_STENCIL_ATTACHMENT);
    blitMask_ |= GL_STENCIL_BUFFER_BIT;
}

void GLRenderTarget::AttachDepthStencilBuffer(const Gs::Vector2ui& size)
{
    AttachRenderBuffer(size, GL_DEPTH_STENCIL, GL_DEPTH_STENCIL_ATTACHMENT);
    blitMask_ |= (GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

static GLenum CheckDrawFramebufferStatus()
{
    return glCheckFramebufferStatus(GL_FRAMEBUFFER);
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

void GLRenderTarget::AttachTexture(Texture& texture, const RenderTargetAttachmentDescriptor& attachmentDesc)
{
    /* Get OpenGL texture object */
    auto& textureGL = LLGL_CAST(GLTexture&, texture);

    /* Apply resolution for MIP-map level */
    auto mipLevel = attachmentDesc.mipLevel;
    ApplyMipResolution(texture, mipLevel);

    GLenum status = 0;
    GLenum attachment = MakeColorAttachment();

    /* If this is a multi-sample render-target but the attachment is not a multi-sample texture, create a multi-sample FBO */
    if (HasMultiSampling() && IsMultiSampleTexture(texture.GetType()))
        CreateOnceFrameBufferMS();

    /* Attach texture to frame buffer (via callback) */
    frameBuffer_.Bind();
    {
        switch (texture.GetType())
        {
            case TextureType::Texture1D:
                GLFrameBuffer::AttachTexture1D(attachment, textureGL, GL_TEXTURE_1D, mipLevel);
                break;
            case TextureType::Texture2D:
                GLFrameBuffer::AttachTexture2D(attachment, textureGL, GL_TEXTURE_2D, mipLevel);
                break;
            case TextureType::Texture3D:
                GLFrameBuffer::AttachTexture3D(attachment, textureGL, GL_TEXTURE_3D, mipLevel, attachmentDesc.layer);
                break;
            case TextureType::TextureCube:
                GLFrameBuffer::AttachTexture2D(attachment, textureGL, GLTypes::Map(attachmentDesc.cubeFace), mipLevel);
                break;
            case TextureType::Texture1DArray:
                GLFrameBuffer::AttachTextureLayer(attachment, textureGL, mipLevel, attachmentDesc.layer);
                break;
            case TextureType::Texture2DArray:
                GLFrameBuffer::AttachTextureLayer(attachment, textureGL, mipLevel, attachmentDesc.layer);
                break;
            case TextureType::TextureCubeArray:
                GLFrameBuffer::AttachTextureLayer(attachment, textureGL, mipLevel, attachmentDesc.layer * 6 + static_cast<int>(attachmentDesc.cubeFace));
                break;
            case TextureType::Texture2DMS:
                GLFrameBuffer::AttachTexture2D(attachment, textureGL, GL_TEXTURE_2D_MULTISAMPLE, 0);
                break;
            case TextureType::Texture2DMSArray:
                GLFrameBuffer::AttachTexture3D(attachment, textureGL, GL_TEXTURE_2D_MULTISAMPLE_ARRAY, 0, attachmentDesc.layer);
                break;
        }

        status = CheckDrawFramebufferStatus();
        
        /* Set draw buffers for this frame buffer if multi-sampling is disabled */
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

void GLRenderTarget::DetachAll()
{
    /* Resets frame buffer and render buffer objects */
    ResetResolution();

    frameBuffer_.Recreate();

    renderBuffer_.reset();
    frameBufferMS_.reset();

    colorAttachments_.clear();
    renderBuffersMS_.clear();

    blitMask_ = 0;
}

/* ----- Extended Internal Functions ----- */

/*
Blit (or rather copy) each multi-sample attachment from the
multi-sample frame buffer (read) into the main frame buffer (draw)
*/
void GLRenderTarget::BlitOntoFrameBuffer()
{
    if (frameBufferMS_)
    {
        frameBuffer_.Bind(GLFrameBufferTarget::DRAW_FRAMEBUFFER);
        frameBufferMS_->Bind(GLFrameBufferTarget::READ_FRAMEBUFFER);

        for (auto attachment : colorAttachments_)
        {
            glReadBuffer(attachment);
            glDrawBuffer(attachment);

            GLFrameBuffer::Blit(GetResolution().Cast<int>(), blitMask_);
        }

        frameBufferMS_->Unbind(GLFrameBufferTarget::READ_FRAMEBUFFER);
        frameBuffer_.Unbind(GLFrameBufferTarget::DRAW_FRAMEBUFFER);
    }
}

/*
Blit (or rather copy) each multi-sample attachment from the
multi-sample frame buffer (read) into the back buffer (draw)
*/
void GLRenderTarget::BlitOntoScreen(std::size_t colorAttachmentIndex)
{
    if (colorAttachmentIndex < colorAttachments_.size())
    {
        GLStateManager::active->BindFrameBuffer(GLFrameBufferTarget::DRAW_FRAMEBUFFER, 0);
        GLStateManager::active->BindFrameBuffer(GLFrameBufferTarget::READ_FRAMEBUFFER, GetFrameBuffer().GetID());
        {
            glReadBuffer(colorAttachments_[colorAttachmentIndex]);
            glDrawBuffer(GL_BACK);

            GLFrameBuffer::Blit(GetResolution().Cast<int>(), blitMask_);
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

void GLRenderTarget::InitRenderBufferStorage(GLRenderBuffer& renderBuffer, GLenum internalFormat)
{
    renderBuffer.Bind();
    {
        GLRenderBuffer::Storage(internalFormat, GetResolution().Cast<int>(), multiSamples_);
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

void GLRenderTarget::AttachRenderBuffer(const Gs::Vector2ui& size, GLenum internalFormat, GLenum attachment)
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

void GLRenderTarget::CreateOnceFrameBufferMS()
{
    if (!frameBufferMS_)
        frameBufferMS_ = MakeUnique<GLFrameBuffer>();
}

bool GLRenderTarget::HasMultiSampling() const
{
    return (multiSamples_ > 1);
}


} // /namespace LLGL



// ================================================================================
