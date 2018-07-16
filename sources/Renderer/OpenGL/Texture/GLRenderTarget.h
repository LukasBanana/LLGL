/*
 * GLRenderTarget.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_GL_RENDER_TARGET_H
#define LLGL_GL_RENDER_TARGET_H


#include <LLGL/RenderTarget.h>
#include "GLFramebuffer.h"
#include "GLRenderbuffer.h"
#include "GLTexture.h"
#include <functional>
#include <vector>
#include <memory>


namespace LLGL
{


class GLRenderTarget final : public RenderTarget
{

    public:

        GLRenderTarget(const RenderTargetDescriptor& desc);

        Extent2D GetResolution() const override;
        std::uint32_t GetNumColorAttachments() const override;

        bool HasDepthAttachment() const override;
        bool HasStencilAttachment() const override;

        const RenderPass* GetRenderPass() const override;

        /* ----- Extended Internal Functions ----- */

        // Blits the multi-sample framebuffer onto the default framebuffer.
        void BlitOntoFramebuffer();

        // Blits the specified color attachment from the framebuffer onto the screen.
        void BlitOntoScreen(std::size_t colorAttachmentIndex);

        // Returns the active framebuffer (i.e. either the default framebuffer or the multi-sample framebuffer).
        const GLFramebuffer& GetFramebuffer() const;

    private:

        void CreateFramebufferWithAttachments(const RenderTargetDescriptor& desc);
        void CreateFramebufferWithNoAttachments(const RenderTargetDescriptor& desc);

        void AttachAllTextures(const std::vector<AttachmentDescriptor>& attachmentDescs, GLenum* internalFormats);
        void AttachAllDepthStencilBuffers(const std::vector<AttachmentDescriptor>& attachmentDescs);

        void AttachDepthBuffer();
        void AttachStencilBuffer();
        void AttachDepthStencilBuffer();
        void AttachTexture(Texture& texture, const AttachmentDescriptor& attachmentDesc, GLenum& internalFormat);

        void InitRenderbufferStorage(GLRenderbuffer& renderbuffer, GLenum internalFormat);

        void CreateAndAttachRenderbuffer(GLenum internalFormat, GLenum attachment);

        GLenum MakeFramebufferAttachment(GLenum internalFormat);

        void CreateRenderbuffersMS(const GLenum* internalFormats);
        void CreateRenderbufferMS(GLenum attachment, GLenum internalFormat);

        // Sets the draw buffers for the currently bound FBO.
        void SetDrawBuffers();

        bool HasMultiSampling() const;
        bool HasCustomMultiSampling() const;
        bool HasDepthStencilAttachment() const;

        void BlitFramebuffer();

        /* === Members === */

        Extent2D                    resolution_;

        GLFramebuffer               framebuffer_;   // primary FBO
        GLFramebuffer               framebufferMS_; // secondary FBO for multi-sampling

        GLRenderbuffer              renderbuffer_;

        /*
        For multi-sampled render targets we also need a renderbuffer for each attached texture.
        Otherwise we would need multi-sampled textures (e.g. "glTexImage2DMultisample")
        which is only supported since OpenGL 3.2+, but renderbuffers are supported since OpenGL 3.0+.
        */
        std::vector<GLRenderbuffer> renderbuffersMS_;

        std::vector<GLenum>         colorAttachments_;

        GLsizei                     multiSamples_       = 0;
        GLbitfield                  blitMask_           = 0;

        const RenderPass*           renderPass_         = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
