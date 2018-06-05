/*
 * DbgRenderTarget.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "DbgRenderTarget.h"
#include "DbgTexture.h"
#include "DbgCore.h"
#include "../CheckedCast.h"


namespace LLGL
{


DbgRenderTarget::DbgRenderTarget(RenderTarget& instance, RenderingDebugger* debugger, const RenderTargetDescriptor& desc) :
    instance { instance },
    desc_    { desc     }
{
    for (const auto& attachment : desc.attachments)
    {
        switch (attachment.type)
        {
            case AttachmentType::Color:
                ++numColorAttachments_;
                break;
            case AttachmentType::Depth:
                hasDepthAttachment_     = true;
                break;
            case AttachmentType::DepthStencil:
                hasDepthAttachment_     = true;
                hasStencilAttachment_   = true;
                break;
            case AttachmentType::Stencil:
                hasStencilAttachment_   = true;
                break;
        }
    }
}


} // /namespace LLGL



// ================================================================================
