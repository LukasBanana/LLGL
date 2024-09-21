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
#include <algorithm>


namespace LLGL
{


GLRenderTarget::GLRenderTarget(const RenderingLimits& limits, const RenderTargetDescriptor& desc) :
    resolution_  { static_cast<GLint>(desc.resolution.width), static_cast<GLint>(desc.resolution.height) },
    drawBuffers_ { SmallVector<GLenum, 2>(std::size_t(NumActiveColorAttachments(desc)))                  },
    samples_     { static_cast<GLint>(GetLimitedRenderTargetSamples(limits, desc))                       },
    renderPass_  { desc.renderPass                                                                       }
{
    framebuffer_.GenFramebuffer();

    if (HasAnyActiveAttachments(desc))
        CreateFramebufferWithAttachments(desc);
    else
        CreateFramebufferWithNoAttachments();

    if (desc.debugName != nullptr)
        SetDebugName(desc.debugName);
}

GLRenderTarget::~GLRenderTarget()
{
    GLStateManager::Get().NotifyGLRenderTargetRelease(this);
}

void GLRenderTarget::SetDebugName(const char* name)
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

bool GLRenderTarget::CanResolveMultisampledFBO() const
{
    return (framebufferResolve_.Valid() && !drawBuffersResolve_.empty());
}

void GLRenderTarget::ResolveMultisampled(GLStateManager& stateMngr)
{
    if (CanResolveMultisampledFBO())
    {
        stateMngr.BindFramebuffer(GLFramebufferTarget::DrawFramebuffer, framebufferResolve_.GetID());
        stateMngr.BindFramebuffer(GLFramebufferTarget::ReadFramebuffer, framebuffer_.GetID());

        #if LLGL_WEBGL
        /* If there are more than one attachment, we have to swap them in and out for WebGL */
        if (drawBuffersResolve_.size() > 1)
        {
            LLGL_ASSERT(resolveAttachments_.size() == drawBuffersResolve_.size());
            for_range(i, drawBuffersResolve_.size())
            {
                glReadBuffer(drawBuffersResolve_[i]);

                const GLFramebufferAttachment& attachment = resolveAttachments_[i];
                GLFramebuffer::AttachTexture(*(attachment.texture), GL_COLOR_ATTACHMENT0, attachment.level, attachment.layer, GL_DRAW_FRAMEBUFFER);

                GLFramebuffer::Blit(resolution_[0], resolution_[1], GL_COLOR_BUFFER_BIT);
            }
        }
        else
        #endif // /LLGL_WEBGL
        {
            for (GLenum buf : drawBuffersResolve_)
            {
                glReadBuffer(buf);
                GLProfile::DrawBuffer(buf);
                GLFramebuffer::Blit(resolution_[0], resolution_[1], GL_COLOR_BUFFER_BIT);
            }
        }

        stateMngr.BindFramebuffer(GLFramebufferTarget::ReadFramebuffer, 0);
        stateMngr.BindFramebuffer(GLFramebufferTarget::DrawFramebuffer, 0);
    }
}

void GLRenderTarget::ResolveMultisampledIntoBackbuffer(GLStateManager& stateMngr, std::uint32_t colorTarget)
{
    if (colorTarget < drawBuffers_.size())
    {
        stateMngr.BindFramebuffer(GLFramebufferTarget::DrawFramebuffer, 0);
        stateMngr.BindFramebuffer(GLFramebufferTarget::ReadFramebuffer, framebuffer_.GetID());
        {
            glReadBuffer(drawBuffers_[colorTarget]);
            GLProfile::DrawBuffer(GL_BACK);
            GLFramebuffer::Blit(resolution_[0], resolution_[1], GL_COLOR_BUFFER_BIT);
        }
        stateMngr.BindFramebuffer(GLFramebufferTarget::ReadFramebuffer, 0);
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
    const GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    GLThrowIfFailed(status, GL_FRAMEBUFFER_COMPLETE, info);
}

void GLRenderTarget::CreateFramebufferWithAttachments(const RenderTargetDescriptor& desc)
{
    const std::uint32_t numColorAttachments = GetNumColorAttachments();

    /* Bind primary FBO */
    GLStateManager::Get().BindFramebuffer(GLFramebufferTarget::DrawFramebuffer, framebuffer_.GetID());
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
    const std::uint32_t numResolveAttachments = NumActiveResolveAttachments(desc);
    if (numResolveAttachments > 0)
    {
        /* Create secondary FBO if standard multi-sampling is enabled */
        framebufferResolve_.GenFramebuffer();

        /* Bind multi-sampled FBO */
        GLStateManager::Get().BindFramebuffer(GLFramebufferTarget::DrawFramebuffer, framebufferResolve_.GetID());
        {
            /* For WebGL, we swap the attachments in and out of GL_COLOR_ATTACHMENT0 binding point if we have more than one attachment */
            #if LLGL_WEBGL
            const bool isAttachmentListSeparated = (numResolveAttachments > 1);
            #else
            constexpr bool isAttachmentListSeparated = false;
            #endif

            /* Attach all color resolve targets */
            for_range(colorTarget, numColorAttachments)
            {
                if (desc.resolveAttachments[colorTarget].texture != nullptr)
                    BuildResolveAttachment(desc.resolveAttachments[colorTarget], colorTarget, isAttachmentListSeparated);
            }

            if (isAttachmentListSeparated)
            {
                /* Set draw buffer only for the first attachment as we swap them in and out at runtime */
                GLProfile::DrawBuffer(drawBuffersResolve_.empty() ? GL_NONE : GL_COLOR_ATTACHMENT0);
            }
            else
            {
                /* Set draw buffers for this framebuffer if multi-sampling is enabled */
                SetGLDrawBuffers(drawBuffersResolve_);
                GLThrowIfFramebufferStatusFailed("color attachments to multi-sample framebuffer object (FBO) failed");
            }
        }
    }
}

void GLRenderTarget::CreateFramebufferWithNoAttachments()
{
    #if LLGL_GLEXT_FRAMEBUFFER_NO_ATTACHMENTS
    if (HasExtension(GLExt::ARB_framebuffer_no_attachments))
    {
        /* Set default framebuffer parameters */
        framebuffer_.FramebufferParameters(resolution_[0], resolution_[1], /*layers:*/ 1, samples_, /*fixedSampleLocations:*/ 0);
    }
    else
    #endif // /LLGL_GLEXT_FRAMEBUFFER_NO_ATTACHMENTS
    {
        /* Bind primary FBO and create dummy renderbuffer attachment */
        GLStateManager::Get().BindFramebuffer(GLFramebufferTarget::DrawFramebuffer, framebuffer_.GetID());
        CreateAndAttachRenderbuffer(GL_COLOR_ATTACHMENT0, GL_RED);
    }

    /* Validate framebuffer status */
    GLThrowIfFramebufferStatusFailed("initializing default parameters for framebuffer object (FBO) failed");
}

void GLRenderTarget::BuildColorAttachment(const AttachmentDescriptor& attachmentDesc, std::uint32_t colorTarget)
{
    const GLenum binding = AllocColorAttachmentBinding(colorTarget);
    if (attachmentDesc.texture != nullptr)
        BuildAttachmentWithTexture(binding, attachmentDesc);
    else
        BuildAttachmentWithRenderbuffer(binding, attachmentDesc.format);
}

void GLRenderTarget::BuildResolveAttachment(const AttachmentDescriptor& attachmentDesc, std::uint32_t colorTarget, bool isAttachmentListSeparated)
{
    LLGL_ASSERT_PTR(attachmentDesc.texture);
    const GLenum binding = AllocResolveAttachmentBinding(colorTarget);
    #if LLGL_WEBGL
    if (isAttachmentListSeparated)
    {
        /* For WebGL, we swap the resolve attachments in and out dynamically to always use GL_COLOR_ATTACHMENT0 binding point */
        GLFramebufferAttachment attachmentGL = {};
        BuildAttachmentWithTexture(binding, attachmentDesc, &attachmentGL);
        resolveAttachments_.push_back(attachmentGL);
    }
    else
    #endif // /LLGL_WEBGL
    {
        /* Attach the texture to the current FBO */
        BuildAttachmentWithTexture(binding, attachmentDesc);
    }
}

void GLRenderTarget::BuildDepthStencilAttachment(const AttachmentDescriptor& attachmentDesc)
{
    if (const Texture* texture = attachmentDesc.texture)
        BuildAttachmentWithTexture(AllocDepthStencilAttachmentBinding(texture->GetFormat()), attachmentDesc);
    else
        BuildAttachmentWithRenderbuffer(AllocDepthStencilAttachmentBinding(attachmentDesc.format), attachmentDesc.format);
}

void GLRenderTarget::BuildAttachmentWithTexture(GLenum binding, const AttachmentDescriptor& attachmentDesc, GLFramebufferAttachment* outAttachmentGL)
{
    LLGL_ASSERT_PTR(attachmentDesc.texture);
    auto* textureGL = LLGL_CAST(GLTexture*, attachmentDesc.texture);

    /* Validate resolution for MIP-map level */
    const std::uint32_t mipLevel = attachmentDesc.mipLevel;
    ValidateMipResolution(*textureGL, mipLevel);

    /* Attach texture to framebuffer */
    if (outAttachmentGL != nullptr)
    {
        outAttachmentGL->texture    = textureGL;
        outAttachmentGL->level      = static_cast<GLint>(mipLevel);
        outAttachmentGL->layer      = static_cast<GLint>(attachmentDesc.arrayLayer);
    }
    else
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
