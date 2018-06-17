/*
 * GLRenderTarget.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLRenderTarget.h"
#include "../RenderState/GLStateManager.h"
#include "../Ext/GLExtensions.h"
#include "../../GLCommon/GLExtensionRegistry.h"
#include "../../CheckedCast.h"
#include "../../GLCommon/GLTypes.h"
#include "../../GLCommon/GLCore.h"
#include "../../../Core/Helper.h"


namespace LLGL
{


[[noreturn]]
static void ErrDepthAttachmentFailed()
{
    throw std::runtime_error("attachment to render target failed, because render target already has a depth-stencil buffer");
}

GLRenderTarget::GLRenderTarget(const RenderTargetDescriptor& desc) :
    RenderTarget  { desc.resolution                                        },
    multiSamples_ { static_cast<GLsizei>(desc.multiSampling.SampleCount()) }
{
    if (HasMultiSampling() && !desc.customMultiSampling)
        CreateOnceFramebufferMS();

    #if 0
    if (desc.attachments.empty())
    {
        //TODO...
    }
    else
    #endif
    {
        /* Initialize all attachments */
        for (const auto& attachment : desc.attachments)
            Attach(attachment);
    }
}

std::uint32_t GLRenderTarget::GetNumColorAttachments() const
{
    return static_cast<std::uint32_t>(colorAttachments_.size());
}

bool GLRenderTarget::HasDepthAttachment() const
{
    return ((blitMask_ & GL_DEPTH_BUFFER_BIT) != 0);
}

bool GLRenderTarget::HasStencilAttachment() const
{
    return ((blitMask_ & GL_STENCIL_BUFFER_BIT) != 0);
}

/* ----- Extended Internal Functions ----- */

//private
void GLRenderTarget::BlitFramebuffer()
{
    GLFramebuffer::Blit(
        static_cast<GLint>(GetResolution().width),
        static_cast<GLint>(GetResolution().height),
        blitMask_
    );
}

/*
Blit (or rather copy) each multi-sample attachment from the
multi-sample framebuffer (read) into the main framebuffer (draw)
*/
void GLRenderTarget::BlitOntoFramebuffer()
{
    if (framebufferMS_)
    {
        framebuffer_.Bind(GLFramebufferTarget::DRAW_FRAMEBUFFER);
        framebufferMS_->Bind(GLFramebufferTarget::READ_FRAMEBUFFER);

        for (auto attachment : colorAttachments_)
        {
            glReadBuffer(attachment);
            glDrawBuffer(attachment);
            BlitFramebuffer();
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
            BlitFramebuffer();
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

void GLRenderTarget::Attach(const AttachmentDescriptor& attachmentDesc)
{
    if (auto texture = attachmentDesc.texture)
    {
        /* Attach texture as color attachment */
        AttachTexture(*texture, attachmentDesc);
    }
    else
    {
        /* Attach (and create) depth-stencil buffer */
        switch (attachmentDesc.type)
        {
            case AttachmentType::Color:
                throw std::invalid_argument("cannot have color attachment in render target without a valid texture");
                break;
            case AttachmentType::Depth:
                AttachDepthBuffer();
                break;
            case AttachmentType::DepthStencil:
                AttachDepthStencilBuffer();
                break;
            case AttachmentType::Stencil:
                AttachStencilBuffer();
                break;
        }
    }
}

void GLRenderTarget::AttachDepthBuffer()
{
    AttachRenderbuffer(GL_DEPTH_COMPONENT, GL_DEPTH_ATTACHMENT);
    blitMask_ |= (GL_DEPTH_BUFFER_BIT);
}

void GLRenderTarget::AttachStencilBuffer()
{
    AttachRenderbuffer(GL_STENCIL_INDEX, GL_STENCIL_ATTACHMENT);
    blitMask_ |= (GL_STENCIL_BUFFER_BIT);
}

void GLRenderTarget::AttachDepthStencilBuffer()
{
    AttachRenderbuffer(GL_DEPTH_STENCIL, GL_DEPTH_STENCIL_ATTACHMENT);
    blitMask_ |= (GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

static GLenum GetFramebufferStatus()
{
    return glCheckFramebufferStatus(GL_FRAMEBUFFER);
}

// Returns the GL internal format for the specified texture object
static GLint GetTexInternalFormat(const GLTexture& textureGL)
{
    GLint internalFormat = GL_RGBA;
    {
        GLStateManager::active->BindTexture(textureGL);
        glGetTexLevelParameteriv(GLTypes::Map(textureGL.GetType()), 0, GL_TEXTURE_INTERNAL_FORMAT, &internalFormat);
    }
    return internalFormat;
}

void GLRenderTarget::AttachTexture(Texture& texture, const AttachmentDescriptor& attachmentDesc)
{
    /* Get OpenGL texture object */
    auto& textureGL = LLGL_CAST(GLTexture&, texture);
    auto textureID = textureGL.GetID();

    /* Validate resolution for MIP-map level */
    auto mipLevel = attachmentDesc.mipLevel;
    ValidateMipResolution(texture, mipLevel);

    /* Make color or depth-stencil attachment */
    const auto internalFormat   = GetTexInternalFormat(textureGL);
    const auto attachment       = MakeFramebufferAttachment(internalFormat);

    /* Attach texture to framebuffer */
    GLenum status = 0;

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
                GLFramebuffer::AttachTextureLayer(attachment, textureID, 0, attachmentDesc.layer);
                break;
        }

        status = GetFramebufferStatus();

        /* Set draw buffers for this framebuffer if multi-sampling is disabled */
        if (!framebufferMS_)
            SetDrawBuffers();
    }
    framebuffer_.Unbind();

    ErrOnIncompleteFramebuffer(status, "color attachment to framebuffer object (FBO) failed");

    /* Create renderbuffer for attachment if multi-sample framebuffer is used */
    if (framebufferMS_)
    {
        auto renderbuffer = MakeUnique<GLRenderbuffer>();
        {
            /* Setup renderbuffer storage by texture's internal format */
            InitRenderbufferStorage(*renderbuffer, static_cast<GLenum>(internalFormat));

            /* Attach renderbuffer to multi-sample framebuffer */
            framebufferMS_->Bind();
            {
                GLFramebuffer::AttachRenderbuffer(attachment, renderbuffer->GetID());
                status = GetFramebufferStatus();

                /* Set draw buffers for this framebuffer is multi-sampling is enabled */
                SetDrawBuffers();
            }
            framebufferMS_->Unbind();

            ErrOnIncompleteFramebuffer(status, "color attachment to multi-sample framebuffer object (FBO) failed");
        }
        renderbuffersMS_.emplace_back(std::move(renderbuffer));
    }
}

void GLRenderTarget::InitRenderbufferStorage(GLRenderbuffer& renderbuffer, GLenum internalFormat)
{
    renderbuffer.Bind();
    {
        GLRenderbuffer::Storage(
            internalFormat,
            static_cast<GLsizei>(GetResolution().width),
            static_cast<GLsizei>(GetResolution().height),
            multiSamples_
        );
    }
    renderbuffer.Unbind();
}

GLenum GLRenderTarget::AttachDefaultRenderbuffer(GLFramebuffer& framebuffer, GLenum attachment)
{
    GLenum status = 0;

    framebuffer.Bind();
    {
        GLFramebuffer::AttachRenderbuffer(attachment, renderbuffer_->GetID());
        status = GetFramebufferStatus();
    }
    framebuffer.Unbind();

    return status;
}

void GLRenderTarget::AttachRenderbuffer(GLenum internalFormat, GLenum attachment)
{
    if (!HasDepthStencilAttachment())
    {
        renderbuffer_ = MakeUnique<GLRenderbuffer>();

        /* Setup renderbuffer storage */
        InitRenderbufferStorage(*renderbuffer_, internalFormat);

        /* Attach renderbuffer to framebuffer (or multi-sample framebuffer if multi-sampling is used) */
        GLenum status = 0;

        if (framebufferMS_)
        {
            status = AttachDefaultRenderbuffer(*framebufferMS_, attachment);
            ErrOnIncompleteFramebuffer(status, "depth- or depth-stencil attachment to multi-sample framebuffer object (FBO) failed");
        }
        else
        {
            status = AttachDefaultRenderbuffer(framebuffer_, attachment);
            ErrOnIncompleteFramebuffer(status, "depth- or depth-stencil attachment to framebuffer object (FBO) failed");
        }
    }
    else
        ErrDepthAttachmentFailed();
}

GLenum GLRenderTarget::MakeFramebufferAttachment(GLint internalFormat)
{
    if (internalFormat == GL_DEPTH_COMPONENT)
    {
        if (!HasDepthStencilAttachment())
        {
            /* Add depth attachment and depth buffer bit to blit mask */
            blitMask_ |= GL_DEPTH_BUFFER_BIT;
            return GL_DEPTH_ATTACHMENT;
        }
        else
            ErrDepthAttachmentFailed();
    }
    else if (internalFormat == GL_DEPTH_STENCIL)
    {
        if (!HasDepthStencilAttachment())
        {
            /* Add depth-stencil attachment and depth-stencil buffer bit to blit mask */
            blitMask_ |= (GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
            return GL_DEPTH_STENCIL_ATTACHMENT;
        }
        else
            ErrDepthAttachmentFailed();
    }
    else
    {
        /* Add color attachment and color buffer bit to blit mask */
        blitMask_ |= GL_COLOR_BUFFER_BIT;
        const GLenum attachment = (GL_COLOR_ATTACHMENT0 + static_cast<GLenum>(colorAttachments_.size()));
        colorAttachments_.push_back(attachment);
        return attachment;
    }
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

void GLRenderTarget::ErrOnIncompleteFramebuffer(const GLenum status, const char* info)
{
    GLThrowIfFailed(status, GL_FRAMEBUFFER_COMPLETE, info);
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

bool GLRenderTarget::HasDepthStencilAttachment() const
{
    static const GLbitfield mask = (GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    return ((blitMask_ & mask) != 0);
}


} // /namespace LLGL



// ================================================================================
