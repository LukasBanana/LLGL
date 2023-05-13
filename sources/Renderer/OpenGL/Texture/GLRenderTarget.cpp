/*
 * GLRenderTarget.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "GLRenderTarget.h"
#include "../GLCore.h"
#include "../GLTypes.h"
#include "../GLObjectUtils.h"
#include "../RenderState/GLStateManager.h"
#include "../Ext/GLExtensions.h"
#include "../Ext/GLExtensionRegistry.h"
#include "../../CheckedCast.h"
#include "../../RenderTargetUtils.h"
#include "../../../Core/CoreUtils.h"
#include "../../../Core/Assertion.h"
#include <LLGL/Utils/ForRange.h>


namespace LLGL
{


GLRenderTarget::GLRenderTarget(const RenderTargetDescriptor& desc) :
    resolution_  { static_cast<GLint>(desc.resolution.width), static_cast<GLint>(desc.resolution.height) },
    drawBuffers_ { SmallVector<GLenum, 2>(std::size_t(NumActiveColorAttachments(desc)))                  },
    samples_     { static_cast<GLint>(desc.samples)                                                      },
    renderPass_  { desc.renderPass                                                                       }
{
    framebuffer_.GenFramebuffer();
    const bool hasAnyAttachments = (!drawBuffers_.empty() || IsAttachmentEnabled(desc.depthStencilAttachment));
    if (hasAnyAttachments)
        CreateFramebufferWithAttachments(desc);
    else
        CreateFramebufferWithNoAttachments();
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
    return Extent2D
    {
        static_cast<std::uint32_t>(resolution_[0]),
        static_cast<std::uint32_t>(resolution_[1])
    };
}

std::uint32_t GLRenderTarget::GetSamples() const
{
    return static_cast<std::uint32_t>(samples_);
}

std::uint32_t GLRenderTarget::GetNumColorAttachments() const
{
    return static_cast<std::uint32_t>(drawBuffers_.size());
}

bool GLRenderTarget::HasDepthAttachment() const
{
    return (depthStencilBinding_ == GL_DEPTH_STENCIL_ATTACHMENT || depthStencilBinding_ == GL_DEPTH_ATTACHMENT);
}

bool GLRenderTarget::HasStencilAttachment() const
{
    return (depthStencilBinding_ == GL_DEPTH_STENCIL_ATTACHMENT || depthStencilBinding_ == GL_STENCIL_ATTACHMENT);
}

const RenderPass* GLRenderTarget::GetRenderPass() const
{
    return renderPass_;
}

void GLRenderTarget::ResolveMultisampled(GLStateManager& stateMngr)
{
    if (framebufferResolve_.Valid() && !drawBuffersResolve_.empty())
    {
        stateMngr.BindFramebuffer(GLFramebufferTarget::DRAW_FRAMEBUFFER, framebufferResolve_.GetID());
        stateMngr.BindFramebuffer(GLFramebufferTarget::READ_FRAMEBUFFER, framebuffer_.GetID());

        for (GLenum buf : drawBuffersResolve_)
        {
            glReadBuffer(buf);
            GLProfile::DrawBuffer(buf);
            GLFramebuffer::Blit(resolution_[0], resolution_[1], GL_COLOR_BUFFER_BIT);
        }

        stateMngr.BindFramebuffer(GLFramebufferTarget::READ_FRAMEBUFFER, 0);
        stateMngr.BindFramebuffer(GLFramebufferTarget::DRAW_FRAMEBUFFER, 0);
    }
}

void GLRenderTarget::ResolveMultisampledIntoBackbuffer(GLStateManager& stateMngr, std::uint32_t colorTarget)
{
    if (colorTarget < drawBuffers_.size())
    {
        stateMngr.BindFramebuffer(GLFramebufferTarget::DRAW_FRAMEBUFFER, 0);
        stateMngr.BindFramebuffer(GLFramebufferTarget::READ_FRAMEBUFFER, framebuffer_.GetID());
        {
            glReadBuffer(drawBuffers_[colorTarget]);
            GLProfile::DrawBuffer(GL_BACK);
            GLFramebuffer::Blit(resolution_[0], resolution_[1], GL_COLOR_BUFFER_BIT);
        }
        stateMngr.BindFramebuffer(GLFramebufferTarget::READ_FRAMEBUFFER, 0);
    }
}

// Sets the draw buffers for the currently bound FBO.
static void SetGLDrawBuffers(const SmallVector<GLenum, 2>& drawBuffers)
{
    /*
    Tell OpenGL which buffers are to be written when drawing operations are performed.
    Each color attachment has its own draw buffer.
    */
    if (drawBuffers.empty())
        GLProfile::DrawBuffer(GL_NONE);
    else if (drawBuffers.size() == 1)
        GLProfile::DrawBuffer(drawBuffers.front());
    else
        glDrawBuffers(static_cast<GLsizei>(drawBuffers.size()), drawBuffers.data());
}

void GLRenderTarget::SetDrawBuffers()
{
    SetGLDrawBuffers(drawBuffers_);
}


/*
 * ======= Private: =======
 */

static void GLThrowIfFramebufferStatusFailed(const char* info)
{
    auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    GLThrowIfFailed(status, GL_FRAMEBUFFER_COMPLETE, info);
}

void GLRenderTarget::CreateFramebufferWithAttachments(const RenderTargetDescriptor& desc)
{
    const std::uint32_t numColorAttachments = GetNumColorAttachments();

    /* Bind primary FBO */
    GLStateManager::Get().BindFramebuffer(GLFramebufferTarget::DRAW_FRAMEBUFFER, framebuffer_.GetID());
    {
        /* Attach all color targets */
        for_range(colorTarget, numColorAttachments)
            BuildColorAttachment(desc.colorAttachments[colorTarget], colorTarget);

        /* Attach depth-stencil targets */
        if (IsAttachmentEnabled(desc.depthStencilAttachment))
            BuildDepthStencilAttachment(desc.depthStencilAttachment);

        /* Finalize primary FBO by setting draw buffers and validate its status */
        SetGLDrawBuffers(drawBuffers_);
        GLThrowIfFramebufferStatusFailed("color attachment to framebuffer object (FBO) failed");
    }

    /* Create secondary FBO if there are any resolve targets */
    if (NumActiveResolveAttachments(desc) > 0)
    {
        /* Create secondary FBO if standard multi-sampling is enabled */
        framebufferResolve_.GenFramebuffer();

        /* Bind multi-sampled FBO */
        GLStateManager::Get().BindFramebuffer(GLFramebufferTarget::DRAW_FRAMEBUFFER, framebufferResolve_.GetID());
        {
            /* Attach all color resolve targets */
            for_range(colorTarget, numColorAttachments)
            {
                if (desc.resolveAttachments[colorTarget].texture != nullptr)
                    BuildResolveAttachment(desc.resolveAttachments[colorTarget], colorTarget);
            }

            /* Set draw buffers for this framebuffer is multi-sampling is enabled */
            SetGLDrawBuffers(drawBuffersResolve_);
            GLThrowIfFramebufferStatusFailed("color attachments to multi-sample framebuffer object (FBO) failed");
        }
    }
}

void GLRenderTarget::CreateFramebufferWithNoAttachments()
{
    #ifdef GL_ARB_framebuffer_no_attachments
    if (HasExtension(GLExt::ARB_framebuffer_no_attachments))
    {
        /* Set default framebuffer parameters */
        framebuffer_.FramebufferParameters(resolution_[0], resolution_[1], /*layers:*/ 1, samples_, /*fixedSampleLocations:*/ 0);
    }
    else
    #endif // /GL_ARB_framebuffer_no_attachments
    {
        /* Bind primary FBO and create dummy renderbuffer attachment */
        GLStateManager::Get().BindFramebuffer(GLFramebufferTarget::DRAW_FRAMEBUFFER, framebuffer_.GetID());
        CreateAndAttachRenderbuffer(GL_COLOR_ATTACHMENT0, GL_RED);
    }

    /* Validate framebuffer status */
    GLThrowIfFramebufferStatusFailed("initializing default parameters for framebuffer object (FBO) failed");
}

void GLRenderTarget::BuildColorAttachment(const AttachmentDescriptor& attachmentDesc, std::uint32_t colorTarget)
{
    const GLenum binding = AllocColorAttachmentBinding(colorTarget);
    if (auto* texture = attachmentDesc.texture)
        BuildAttachmentWithTexture(binding, attachmentDesc);
    else
        BuildAttachmentWithRenderbuffer(binding, attachmentDesc.format);
}

void GLRenderTarget::BuildResolveAttachment(const AttachmentDescriptor& attachmentDesc, std::uint32_t colorTarget)
{
    LLGL_ASSERT_PTR(attachmentDesc.texture);
    BuildAttachmentWithTexture(AllocResolveAttachmentBinding(colorTarget), attachmentDesc);
}

void GLRenderTarget::BuildDepthStencilAttachment(const AttachmentDescriptor& attachmentDesc)
{
    if (auto* texture = attachmentDesc.texture)
        BuildAttachmentWithTexture(AllocDepthStencilAttachmentBinding(texture->GetFormat()), attachmentDesc);
    else
        BuildAttachmentWithRenderbuffer(AllocDepthStencilAttachmentBinding(attachmentDesc.format), attachmentDesc.format);
}

void GLRenderTarget::BuildAttachmentWithTexture(GLenum binding, const AttachmentDescriptor& attachmentDesc)
{
    LLGL_ASSERT_PTR(attachmentDesc.texture);
    auto* textureGL = LLGL_CAST(GLTexture*, attachmentDesc.texture);

    /* Validate resolution for MIP-map level */
    auto mipLevel = attachmentDesc.mipLevel;
    ValidateMipResolution(*textureGL, mipLevel);

    /* Attach texture to framebuffer */
    GLFramebuffer::AttachTexture(*textureGL, binding, static_cast<GLint>(mipLevel), static_cast<GLint>(attachmentDesc.arrayLayer));
}

void GLRenderTarget::BuildAttachmentWithRenderbuffer(GLenum binding, Format format)
{
    CreateAndAttachRenderbuffer(binding, GLTypes::Map(format));
}

void GLRenderTarget::CreateAndAttachRenderbuffer(GLenum binding, GLenum internalFormat)
{
    GLRenderbuffer renderbuffer;
    {
        renderbuffer.GenRenderbuffer();
        renderbuffer.BindAndAllocStorage(internalFormat, resolution_[0], resolution_[1], samples_);
        GLFramebuffer::AttachRenderbuffer(binding, renderbuffer.GetID());
    }
    renderbuffers_.push_back(std::move(renderbuffer));
}

GLenum GLRenderTarget::AllocColorAttachmentBinding(std::uint32_t colorTarget)
{
    LLGL_ASSERT(colorTarget < drawBuffers_.size());
    const GLenum drawBuffer = GLTypes::ToColorAttachment(colorTarget);
    drawBuffers_[colorTarget] = drawBuffer;
    return drawBuffer;
}

GLenum GLRenderTarget::AllocResolveAttachmentBinding(std::uint32_t colorTarget)
{
    const GLenum drawBuffer = GLTypes::ToColorAttachment(colorTarget);
    drawBuffersResolve_.push_back(drawBuffer);
    return drawBuffer;
}

static GLenum ToGLDepthStencilAttachmentBinding(const Format format)
{
    if (IsDepthAndStencilFormat(format))
        return GL_DEPTH_STENCIL_ATTACHMENT;
    else if (IsDepthFormat(format))
        return GL_DEPTH_ATTACHMENT;
    else if (IsStencilFormat(format))
        return GL_STENCIL_ATTACHMENT;
    LLGL_UNREACHABLE();
}

GLenum GLRenderTarget::AllocDepthStencilAttachmentBinding(const Format format)
{
    LLGL_ASSERT(depthStencilBinding_ == 0);

    const GLenum binding = ToGLDepthStencilAttachmentBinding(format);
    depthStencilBinding_ = binding;

    return binding;
}


} // /namespace LLGL



// ================================================================================
