/*
 * DbgRenderTarget.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
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

        RenderTarget&           instance;

    private:

        RenderingDebugger*      debugger_   = nullptr;
        RenderTargetDescriptor  desc_;

};


} // /namespace LLGL


#endif



// ================================================================================
