/*
 * DbgRenderTarget.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_DBG_RENDER_TARGET_H__
#define __LLGL_DBG_RENDER_TARGET_H__


#include <LLGL/RenderTarget.h>


namespace LLGL
{


class DbgRenderTarget : public RenderTarget
{

    public:

        DbgRenderTarget(RenderTarget& instance, unsigned int multiSamples);

        void AttachDepthBuffer(const Gs::Vector2i& size) override;
        void AttachStencilBuffer(const Gs::Vector2i& size) override;
        void AttachDepthStencilBuffer(const Gs::Vector2i& size) override;

        void AttachTexture(Texture& texture, const RenderTargetAttachmentDescriptor& attachmentDesc) override;

        void DetachAll() override;

        RenderTarget& instance;

};


} // /namespace LLGL


#endif



// ================================================================================
