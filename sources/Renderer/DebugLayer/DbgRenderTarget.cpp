/*
 * DbgRenderTarget.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "DbgRenderTarget.h"
#include "DbgTexture.h"
#include "DbgCore.h"
#include "../CheckedCast.h"


namespace LLGL
{


DbgRenderTarget::DbgRenderTarget(RenderTarget& instance, RenderingDebugger* debugger, const RenderTargetDescriptor& desc) :
    instance    ( instance ),
    debugger_   ( debugger ),
    desc_       ( desc     )
{
}

void DbgRenderTarget::AttachDepthBuffer(const Gs::Vector2ui& size)
{
    instance.AttachDepthBuffer(size);
}

void DbgRenderTarget::AttachStencilBuffer(const Gs::Vector2ui& size)
{
    instance.AttachStencilBuffer(size);
}

void DbgRenderTarget::AttachDepthStencilBuffer(const Gs::Vector2ui& size)
{
    instance.AttachDepthStencilBuffer(size);
}

void DbgRenderTarget::AttachTexture(Texture& texture, const RenderTargetAttachmentDescriptor& attachmentDesc)
{
    auto& textureDbg = LLGL_CAST(DbgTexture&, texture);

    if (debugger_)
    {
        LLGL_DBG_SOURCE;

        if (desc_.multiSampling.SampleCount() > 1 && desc_.customMultiSampling && !IsMultiSampleTexture(texture.GetType()))
            LLGL_DBG_ERROR(ErrorType::InvalidArgument, "attempt to attach non-multi-sample texture to render-target with custom multi-sampling");
    }

    instance.AttachTexture(textureDbg.instance, attachmentDesc);
}

void DbgRenderTarget::DetachAll()
{
    instance.DetachAll();
}


} // /namespace LLGL



// ================================================================================
