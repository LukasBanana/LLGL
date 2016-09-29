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


DbgRenderTarget::DbgRenderTarget(RenderTarget& instance, unsigned int multiSamples) :
    instance( instance )
{
}

void DbgRenderTarget::AttachDepthBuffer(const Gs::Vector2i& size)
{
    instance.AttachDepthBuffer(size);
}

void DbgRenderTarget::AttachStencilBuffer(const Gs::Vector2i& size)
{
    instance.AttachStencilBuffer(size);
}

void DbgRenderTarget::AttachDepthStencilBuffer(const Gs::Vector2i& size)
{
    instance.AttachDepthStencilBuffer(size);
}

void DbgRenderTarget::AttachTexture(Texture& texture, const RenderTargetAttachmentDescriptor& attachmentDesc)
{
    auto& textureDbg = LLGL_CAST(DbgTexture&, texture);
    instance.AttachTexture(textureDbg.instance, attachmentDesc);
}

void DbgRenderTarget::DetachAll()
{
    instance.DetachAll();
}


} // /namespace LLGL



// ================================================================================
