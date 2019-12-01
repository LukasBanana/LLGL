/*
 * GLRenderTarget.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLRenderTarget.h"
#include "../GLCore.h"
#include "../GLTypes.h"
#include "../GLObjectUtils.h"
#include "../RenderState/GLStateManager.h"
#include "../Ext/GLExtensions.h"
#include "../Ext/GLExtensionRegistry.h"
#include "../../CheckedCast.h"
#include "../../../Core/Helper.h"


namespace LLGL
{


/*
 * Internals
 */

static const std::size_t g_maxFramebufferAttachments = 32;

[[noreturn]]
static void ErrDepthAttachmentFailed()
{
    throw std::runtime_error("attachment to render target failed, because render target already has a depth-stencil buffer");
}

[[noreturn]]
static void ErrTooManyColorAttachments(std::size_t numColorAttchments)
{
    throw std::runtime_error(
        "too many color attachments for render target (" +
        std::to_string(numColorAttchments) + " is specified, but limit is " +  std::to_string(g_maxFramebufferAttachments) + ")"
    );
}

static void ValidateFramebufferStatus(const char* info)
{
    auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    GLThrowIfFailed(status, GL_FRAMEBUFFER_COMPLETE, info);
}

static std::size_t CountColorAttachments(const std::vector<AttachmentDescriptor>& attachmentDescs)
{
    std::size_t numColorAttachments = 0;

    for (const auto& attachmentDesc : attachmentDescs)
    {
        if (attachmentDesc.type == AttachmentType::Color)
            ++numColorAttachments;
    }

    if (numColorAttachments > g_maxFramebufferAttachments)
        ErrTooManyColorAttachments(g_maxFramebufferAttachments);

    return numColorAttachments;
}


/*
 * GLRenderTarget class
 */

GLRenderTarget::GLRenderTarget(const RenderTargetDescriptor& desc) :
    resolution_ { desc.resolution                    },
    samples_    { static_cast<GLsizei>(desc.samples) },
    renderPass_ { desc.renderPass                    }
{
    framebuffer_.GenFramebuffer();
    if (desc.attachments.empty())
        CreateFramebufferWithNoAttachments(desc);
    else
        CreateFramebufferWithAttachments(desc);
}

GLRenderTarget::~GLRenderTarget()
{
    GLStateManager::Get().NotifyGLRenderTargetRelease(this);
}

void GLRenderTarget::SetName(const char* name)
{
    GLSetObjectLabel(GL_FRAMEBUFFER, framebuffer_.GetID(), name);
}

Extent2D GLRenderTarget::GetResolution() const
{
    return resolution_;
}

std::uint32_t GLRenderTarget::GetSamples() const
{
    return static_cast<std::uint32_t>(samples_);
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

const RenderPass* GLRenderTarget::GetRenderPass() const
{
    return renderPass_;
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
    if (framebufferMS_ && !colorAttachments_.empty())
    {
        framebuffer_.Bind(GLFramebufferTarget::DRAW_FRAMEBUFFER);
        framebufferMS_.Bind(GLFramebufferTarget::READ_FRAMEBUFFER);

        for (auto attachment : colorAttachments_)
        {
            glReadBuffer(attachment);
            GLProfile::DrawBuffer(attachment);
            BlitFramebuffer();
        }

        framebufferMS_.Unbind(GLFramebufferTarget::READ_FRAMEBUFFER);
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
        GLStateManager::Get().BindFramebuffer(GLFramebufferTarget::DRAW_FRAMEBUFFER, 0);
        GLStateManager::Get().BindFramebuffer(GLFramebufferTarget::READ_FRAMEBUFFER, GetFramebuffer().GetID());
        {
            glReadBuffer(colorAttachments_[colorAttachmentIndex]);
            GLProfile::DrawBuffer(GL_BACK);
            BlitFramebuffer();
        }
        GLStateManager::Get().BindFramebuffer(GLFramebufferTarget::READ_FRAMEBUFFER, 0);
    }
}

const GLFramebuffer& GLRenderTarget::GetFramebuffer() const
{
    return (framebufferMS_.Valid() ? framebufferMS_ : framebuffer_);
}

void GLRenderTarget::SetDrawBuffers()
{
    /*
    Tell OpenGL which buffers are to be written when drawing operations are performed.
    Each color attachment has its own draw buffer.
    */
    if (colorAttachments_.empty())
        GLProfile::DrawBuffer(GL_NONE);
    else if (colorAttachments_.size() == 1)
        GLProfile::DrawBuffer(colorAttachments_.front());
    else
        glDrawBuffers(static_cast<GLsizei>(colorAttachments_.size()), colorAttachments_.data());
}


/*
 * ======= Private: =======
 */

void GLRenderTarget::CreateFramebufferWithAttachments(const RenderTargetDescriptor& desc)
{
    /* Create secondary FBO if standard multi-sampling is enabled */
    if (HasMultiSampling() && !desc.customMultiSampling)
        framebufferMS_.GenFramebuffer();

    /* Determine number of color attachments */
    GLenum internalFormats[g_maxFramebufferAttachments];
    auto numColorAttachments = CountColorAttachments(desc.attachments);

    /* Reserve storage for color attachment slots */
    colorAttachments_.reserve(numColorAttachments);

    /* Bind primary FBO */
    GLStateManager::Get().BindFramebuffer(GLFramebufferTarget::DRAW_FRAMEBUFFER, framebuffer_.GetID());
    {
        if (framebufferMS_)
        {
            /* Only attach textures (renderbuffers are only attached to multi-sampled FBO) */
            AttachAllTextures(desc.attachments, internalFormats);
        }
        else
        {
            /* Attach all depth-stencil buffers and textures if multi-sampling is disabled */
            AttachAllDepthStencilBuffers(desc.attachments);
            AttachAllTextures(desc.attachments, internalFormats);
            SetDrawBuffers();
        }

        /* Validate framebuffer status */
        ValidateFramebufferStatus("color attachment to framebuffer object (FBO) failed");
    }

    /* Create renderbuffers for multi-sampled render-target */
    if (framebufferMS_)
    {
        /* Bind multi-sampled FBO */
        GLStateManager::Get().BindFramebuffer(GLFramebufferTarget::DRAW_FRAMEBUFFER, framebufferMS_.GetID());
        {
            /* Create depth-stencil attachmnets */
            AttachAllDepthStencilBuffers(desc.attachments);

            /* Create all renderbuffers as storage source for multi-sampled render target */
            CreateRenderbuffersMS(internalFormats);
        }
    }
}

void GLRenderTarget::CreateFramebufferWithNoAttachments(const RenderTargetDescriptor& desc)
{
    #ifdef GL_ARB_framebuffer_no_attachments
    if (HasExtension(GLExt::ARB_framebuffer_no_attachments))
    {
        /* Set default framebuffer parameters */
        framebuffer_.FramebufferParameters(
            static_cast<GLint>(desc.resolution.width),
            static_cast<GLint>(desc.resolution.height),
            1,
            static_cast<GLint>(samples_),
            0
        );
    }
    else
    #endif // /GL_ARB_framebuffer_no_attachments
    {
        /* Bind primary FBO */
        GLStateManager::Get().BindFramebuffer(GLFramebufferTarget::DRAW_FRAMEBUFFER, framebuffer_.GetID());

        /* Create dummy renderbuffer attachment */
        renderbuffer_.GenRenderbuffer();
        renderbuffer_.BindAndAllocStorage(
            GL_RED,
            static_cast<GLsizei>(desc.resolution.width),
            static_cast<GLsizei>(desc.resolution.height),
            samples_
        );

        /* Attach dummy renderbuffer to first color attachment slot */
        framebuffer_.AttachRenderbuffer(GL_COLOR_ATTACHMENT0, renderbuffer_.GetID());
    }

    /* Validate framebuffer status */
    ValidateFramebufferStatus("initializing default parameters for framebuffer object (FBO) failed");
}

void GLRenderTarget::AttachAllTextures(const std::vector<AttachmentDescriptor>& attachmentDescs, GLenum* internalFormats)
{
    std::size_t colorAttachmentIndex = 0;

    for (const auto& attachmentDesc : attachmentDescs)
    {
        if (auto texture = attachmentDesc.texture)
        {
            /* Attach texture as color attachment */
            AttachTexture(*texture, attachmentDesc, internalFormats[colorAttachmentIndex++]);
        }
    }
}

void GLRenderTarget::AttachAllDepthStencilBuffers(const std::vector<AttachmentDescriptor>& attachmentDescs)
{
    for (const auto& attachmentDesc : attachmentDescs)
    {
        if (attachmentDesc.texture == nullptr)
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
}

void GLRenderTarget::AttachDepthBuffer()
{
    CreateAndAttachRenderbuffer(GL_DEPTH_COMPONENT, GL_DEPTH_ATTACHMENT);
    blitMask_ |= (GL_DEPTH_BUFFER_BIT);
}

void GLRenderTarget::AttachStencilBuffer()
{
    CreateAndAttachRenderbuffer(GL_STENCIL_INDEX, GL_STENCIL_ATTACHMENT);
    blitMask_ |= (GL_STENCIL_BUFFER_BIT);
}

void GLRenderTarget::AttachDepthStencilBuffer()
{
    CreateAndAttachRenderbuffer(GL_DEPTH_STENCIL, GL_DEPTH_STENCIL_ATTACHMENT);
    blitMask_ |= (GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void GLRenderTarget::AttachTexture(Texture& texture, const AttachmentDescriptor& attachmentDesc, GLenum& internalFormat)
{
    /* Get OpenGL texture object */
    auto& textureGL = LLGL_CAST(GLTexture&, texture);

    /* Validate resolution for MIP-map level */
    auto mipLevel = attachmentDesc.mipLevel;
    ValidateMipResolution(texture, mipLevel);

    /* Store internal texture format into output parameter */
    internalFormat = textureGL.GetGLInternalFormat();

    /* Make color or depth-stencil attachment */
    auto attachment = MakeFramebufferAttachment(attachmentDesc.type);

    /* Attach texture to framebuffer */
    GLFramebuffer::AttachTexture(textureGL, attachment, static_cast<GLint>(mipLevel), static_cast<GLint>(attachmentDesc.arrayLayer));
}

void GLRenderTarget::CreateRenderbuffersMS(const GLenum* internalFormats)
{
    /* Create renderbuffer for attachment if multi-sample framebuffer is used */
    renderbuffersMS_.reserve(colorAttachments_.size());

    /* Create alll renderbuffers as storage for multi-sampled attachments */
    for (std::size_t i = 0; i < colorAttachments_.size(); ++i)
        CreateRenderbufferMS(colorAttachments_[i], internalFormats[i]);

    /* Set draw buffers for this framebuffer is multi-sampling is enabled */
    SetDrawBuffers();

    /* Validate framebuffer status */
    ValidateFramebufferStatus("color attachments to multi-sample framebuffer object (FBO) failed");
}

void GLRenderTarget::CreateRenderbufferMS(GLenum attachment, GLenum internalFormat)
{
    GLRenderbuffer renderbuffer;
    renderbuffer.GenRenderbuffer();
    {
        /* Setup renderbuffer storage by texture's internal format */
        InitRenderbufferStorage(renderbuffer, internalFormat);

        /* Attach renderbuffer to multi-sample framebuffer */
        GLFramebuffer::AttachRenderbuffer(attachment, renderbuffer.GetID());
    }
    renderbuffersMS_.emplace_back(std::move(renderbuffer));
}

void GLRenderTarget::InitRenderbufferStorage(GLRenderbuffer& renderbuffer, GLenum internalFormat)
{
    renderbuffer.BindAndAllocStorage(
        internalFormat,
        static_cast<GLsizei>(GetResolution().width),
        static_cast<GLsizei>(GetResolution().height),
        samples_
    );
}

void GLRenderTarget::CreateAndAttachRenderbuffer(GLenum internalFormat, GLenum attachment)
{
    if (!renderbuffer_)
    {
        /* Create renderbuffer for depth-stencil attachment */
        renderbuffer_.GenRenderbuffer();

        /* Setup renderbuffer storage */
        InitRenderbufferStorage(renderbuffer_, internalFormat);

        /* Attach renderbuffer to framebuffer (or multi-sample framebuffer if multi-sampling is used) */
        GLFramebuffer::AttachRenderbuffer(attachment, renderbuffer_.GetID());
    }
    else
        ErrDepthAttachmentFailed();
}

GLenum GLRenderTarget::MakeFramebufferAttachment(const AttachmentType type)
{
    /* Check if there is no depth-stencil attachment yet */
    switch (type)
    {
        case AttachmentType::Color:         return MakeFramebufferColorAttachment();
        case AttachmentType::Depth:         return MakeFramebufferDepthStencilAttachment(true, false);
        case AttachmentType::DepthStencil:  return MakeFramebufferDepthStencilAttachment(true, true);
        case AttachmentType::Stencil:       return MakeFramebufferDepthStencilAttachment(false, true);
    }
    return 0;
}

GLenum GLRenderTarget::MakeFramebufferColorAttachment()
{
    /* Add color attachment and color buffer bit to blit mask */
    blitMask_ |= GL_COLOR_BUFFER_BIT;

    const GLenum attachment = GLTypes::ToColorAttachment(static_cast<std::uint32_t>(colorAttachments_.size()));
    colorAttachments_.push_back(attachment);

    return attachment;
}

GLenum GLRenderTarget::MakeFramebufferDepthStencilAttachment(bool depth, bool stencil)
{
    if (!HasDepthStencilAttachment())
    {
        if (depth && stencil)
        {
            /* Add depth-stencil attachment and depth-stencil buffer bit to blit mask */
            blitMask_ |= (GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
            return GL_DEPTH_STENCIL_ATTACHMENT;
        }
        else if (depth)
        {
            /* Add depth attachment and depth buffer bit to blit mask */
            blitMask_ |= GL_DEPTH_BUFFER_BIT;
            return GL_DEPTH_ATTACHMENT;
        }
        else if (stencil)
        {
            /* Add depth-stencil attachment but only stenicl buffer bit to blit mask */
            blitMask_ |= GL_STENCIL_BUFFER_BIT;
            return GL_DEPTH_STENCIL_ATTACHMENT;
        }
    }
    else
        ErrDepthAttachmentFailed();
    return 0;
}

bool GLRenderTarget::HasMultiSampling() const
{
    return (samples_ > 1);
}

bool GLRenderTarget::HasCustomMultiSampling() const
{
    return (framebufferMS_.Valid());
}

bool GLRenderTarget::HasDepthStencilAttachment() const
{
    static const GLbitfield mask = (GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    return ((blitMask_ & mask) != 0);
}


} // /namespace LLGL



// ================================================================================
