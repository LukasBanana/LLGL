/*
 * DbgRenderTarget.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_DBG_RENDER_TARGET_H
#define LLGL_DBG_RENDER_TARGET_H


#include <LLGL/RenderTarget.h>


namespace LLGL
{


class RenderingDebugger;

class DbgRenderTarget : public RenderTarget
{

    public:

        DbgRenderTarget(RenderTarget& instance, RenderingDebugger* debugger, const RenderTargetDescriptor& desc);

        void AttachDepthBuffer(const Gs::Vector2ui& size) override;
        void AttachStencilBuffer(const Gs::Vector2ui& size) override;
        void AttachDepthStencilBuffer(const Gs::Vector2ui& size) override;

        void AttachTexture(Texture& texture, const RenderTargetAttachmentDescriptor& attachmentDesc) override;

        void DetachAll() override;

        // Returns the render target descriptor of this debug layer object.
        inline const RenderTargetDescriptor& GetDesc() const
        {
            return desc_;
        }

        // Returns the number of color attachments this render target has.
        inline std::uint32_t GetNumColorAttachments() const
        {
            return numColorAttachments_;
        }

        // Returns true if this render target has a depth attachment.
        inline bool HasDepthAttachment() const
        {
            return hasDepthAttachment_;
        }

        // Returns true if this render target has a stencil attachment.
        inline bool HasStencilAttachment() const
        {
            return hasStencilAttachment_;
        }

        RenderTarget&           instance;

    private:

        RenderingDebugger*      debugger_               = nullptr;
        RenderTargetDescriptor  desc_;
        std::uint32_t           numColorAttachments_    = 0;
        bool                    hasDepthAttachment_     = false;
        bool                    hasStencilAttachment_   = false;

};


} // /namespace LLGL


#endif



// ================================================================================
