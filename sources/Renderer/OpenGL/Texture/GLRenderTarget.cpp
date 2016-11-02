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
#include "../../GLCommon/GLTypes.h"
#include "../../GLCommon/GLCore.h"
#include "../../../Core/Helper.h"


namespace LLGL
{


GLRenderTarget::GLRenderTarget(const RenderTargetDescriptor& desc) :
    multiSamples_( static_cast<GLsizei>(desc.multiSampling.SampleCount()) )
{
    if (HasMultiSampling() && !desc.customMultiSampling)
        CreateOnceFramebufferMS();
}

void GLRenderTarget::AttachDepthBuffer(const Gs::Vector2ui& size)
{
    AttachRenderbuffer(size, GL_DEPTH_COMPONENT, GL_DEPTH_ATTACHMENT);
    blitMask_ |= GL_DEPTH_BUFFER_BIT;
}

void GLRenderTarget::AttachStencilBuffer(const Gs::Vector2ui& size)
{
    AttachRenderbuffer(size, GL_STENCIL_INDEX, GL_STENCIL_ATTACHMENT);
    blitMask_ |= GL_STENCIL_BUFFER_BIT;
}

void GLRenderTarget::AttachDepthStencilBuffer(const Gs::Vector2ui& size)
{
    AttachRenderbuffer(size, GL_DEPTH_STENCIL, GL_DEPTH_STENCIL_ATTACHMENT);
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
    auto textureID = textureGL.GetID();

    /* Apply resolution for MIP-map level */
    auto mipLevel = attachmentDesc.mipLevel;
    ApplyMipResolution(texture, mipLevel);

    GLenum status = 0;
    GLenum attachment = MakeColorAttachment();

    /* Attach texture to framebuffer (via callback) */
    framebuffer_.Bind();
    {
        switch (texture.GetType())
        {
            case TextureType::Texture1D:
                GLFramebuffer::AttachTexture1D(attachment, GL_TEXTURE_1D, textureID, mipLevel);
                break;
            case TextureType::Texture2D:
                GLFramebuffer::AttachTexture2D(attachment, GL_TEXTURE_2D, textureID, mipLevel);
                break;
            case TextureType::Texture3D:
                GLFramebuffer::AttachTexture3D(attachment, GL_TEXTURE_3D, textureID, mipLevel, attachmentDesc.layer);
                break;
            case TextureType::TextureCube:
                GLFramebuffer::AttachTexture2D(attachment, GLTypes::Map(attachmentDesc.cubeFace), textureID, mipLevel);
                break;
            case TextureType::Texture1DArray:
                GLFramebuffer::AttachTextureLayer(attachment, textureID, mipLevel, attachmentDesc.layer);
                break;
            case TextureType::Texture2DArray:
                GLFramebuffer::AttachTextureLayer(attachment, textureID, mipLevel, attachmentDesc.layer);
                break;
            case TextureType::TextureCubeArray:
                GLFramebuffer::AttachTextureLayer(attachment, textureID, mipLevel, attachmentDesc.layer * 6 + static_cast<int>(attachmentDesc.cubeFace));
                break;
            case TextureType::Texture2DMS:
                GLFramebuffer::AttachTexture2D(attachment, GL_TEXTURE_2D_MULTISAMPLE, textureID, 0);
                break;
            case TextureType::Texture2DMSArray:
                GLFramebuffer::AttachTexture3D(attachment, GL_TEXTURE_2D_MULTISAMPLE_ARRAY, textureID, 0, attachmentDesc.layer);
                break;
        }

        status = CheckDrawFramebufferStatus();
        
        /* Set draw buffers for this framebuffer if multi-sampling is disabled */
        if (!framebufferMS_)
            SetDrawBuffers();
    }
    framebuffer_.Unbind();

    CheckFramebufferStatus(status, "color attachment to framebuffer object (FBO)");

    /* Create renderbuffer for attachment if multi-sample framebuffer is used */
    if (framebufferMS_)
    {
        auto renderbuffer = MakeUnique<GLRenderbuffer>();
        {
            /* Setup renderbuffer storage by texture's internal format */
            InitRenderbufferStorage(*renderbuffer, GetTexInternalFormat(textureGL));

            /* Attach renderbuffer to multi-sample framebuffer */
            framebufferMS_->Bind();
            {
                GLFramebuffer::AttachRenderbuffer(attachment, renderbuffer->GetID());
                status = CheckDrawFramebufferStatus();
                
                /* Set draw buffers for this framebuffer is multi-sampling is enabled */
                SetDrawBuffers();
            }
            framebufferMS_->Unbind();

            CheckFramebufferStatus(status, "color attachment to multi-sample framebuffer object (FBO)");
        }
        renderbuffersMS_.emplace_back(std::move(renderbuffer));
    }

    /* Add color buffer bit to blit mask */
    blitMask_ |= GL_COLOR_BUFFER_BIT;
}

void GLRenderTarget::DetachAll()
{
    /* Resets framebuffer and renderbuffer objects */
    ResetResolution();

    framebuffer_.Recreate();

    renderbuffer_.reset();

    if (framebufferMS_)
        framebufferMS_->Recreate();

    colorAttachments_.clear();
    renderbuffersMS_.clear();

    blitMask_ = 0;
}

/* ----- Extended Internal Functions ----- */

/*
Blit (or rather copy) each multi-sample attachment from the
multi-sample framebuffer (read) into the main framebuffer (draw)
*/
void GLRenderTarget::BlitOntoFrameBuffer()
{
    if (framebufferMS_)
    {
        framebuffer_.Bind(GLFramebufferTarget::DRAW_FRAMEBUFFER);
        framebufferMS_->Bind(GLFramebufferTarget::READ_FRAMEBUFFER);

        for (auto attachment : colorAttachments_)
        {
            glReadBuffer(attachment);
            glDrawBuffer(attachment);

            GLFramebuffer::Blit(GetResolution().Cast<int>(), blitMask_);
        }

        framebufferMS_->Unbind(GLFramebufferTarget::READ_FRAMEBUFFER);
        framebuffer_.Unbind(GLFramebufferTarget::DRAW_FRAMEBUFFER);
    }
}

/*
Blit (or rather copy) each multi-sample attachment from the
multi-sample framebuffer (read) into the back buffer (draw)
*/
void GLRenderTarget::BlitOntoScreen(std::size_t colorAttachmentIndex)
{
    if (colorAttachmentIndex < colorAttachments_.size())
    {
        GLStateManager::active->BindFramebuffer(GLFramebufferTarget::DRAW_FRAMEBUFFER, 0);
        GLStateManager::active->BindFramebuffer(GLFramebufferTarget::READ_FRAMEBUFFER, GetFramebuffer().GetID());
        {
            glReadBuffer(colorAttachments_[colorAttachmentIndex]);
            glDrawBuffer(GL_BACK);

            GLFramebuffer::Blit(GetResolution().Cast<int>(), blitMask_);
        }
        GLStateManager::active->BindFramebuffer(GLFramebufferTarget::READ_FRAMEBUFFER, 0);
    }
}

const GLFramebuffer& GLRenderTarget::GetFramebuffer() const
{
    return (framebufferMS_ != nullptr ? *framebufferMS_ : framebuffer_);
}


/*
 * ======= Private: =======
 */

void GLRenderTarget::InitRenderbufferStorage(GLRenderbuffer& renderbuffer, GLenum internalFormat)
{
    renderbuffer.Bind();
    {
        GLRenderbuffer::Storage(internalFormat, GetResolution().Cast<int>(), multiSamples_);
    }
    renderbuffer.Unbind();
}

GLenum GLRenderTarget::AttachDefaultRenderbuffer(GLFramebuffer& framebuffer, GLenum attachment)
{
    GLenum status = 0;

    framebuffer.Bind();
    {
        GLFramebuffer::AttachRenderbuffer(attachment, renderbuffer_->GetID());
        status = CheckDrawFramebufferStatus();
    }
    framebuffer.Unbind();

    return status;
}

void GLRenderTarget::AttachRenderbuffer(const Gs::Vector2ui& size, GLenum internalFormat, GLenum attachment)
{
    if (!renderbuffer_)
    {
        renderbuffer_ = MakeUnique<GLRenderbuffer>();

        /* Apply size to framebuffer resolution */
        ApplyResolution(size);

        /* Setup renderbuffer storage */
        InitRenderbufferStorage(*renderbuffer_, internalFormat);

        /* Attach renderbuffer to framebuffer (or multi-sample framebuffer if multi-sampling is used) */
        GLenum status = 0;

        if (framebufferMS_)
        {
            status = AttachDefaultRenderbuffer(*framebufferMS_, attachment);
            CheckFramebufferStatus(status, "depth- or depth-stencil attachment to multi-sample framebuffer object (FBO)");
        }
        else
        {
            status = AttachDefaultRenderbuffer(framebuffer_, attachment);
            CheckFramebufferStatus(status, "depth- or depth-stencil attachment to framebuffer object (FBO)");
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

void GLRenderTarget::CheckFramebufferStatus(GLenum status, const std::string& info)
{
    if (status != GL_FRAMEBUFFER_COMPLETE)
        throw std::runtime_error(info + " failed (error code = " + GLErrorToStr(status) + ")");
}

void GLRenderTarget::CreateOnceFramebufferMS()
{
    if (!framebufferMS_)
        framebufferMS_ = MakeUnique<GLFramebuffer>();
}

bool GLRenderTarget::HasMultiSampling() const
{
    return (multiSamples_ > 1);
}

bool GLRenderTarget::HasCustomMultiSampling() const
{
    return (framebufferMS_ == nullptr);
}


} // /namespace LLGL



// ================================================================================
