/*
 * DbgRenderPass.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "DbgRenderPass.h"
#include "../DbgCore.h"
#include "../DbgSwapChain.h"
#include "../../RenderPassUtils.h"


namespace LLGL
{


DbgRenderPass::DbgRenderPass(RenderPass& instance, const RenderPassDescriptor& desc) :
    instance        { instance             },
    mutableInstance { &instance            },
    desc            { desc                 },
    label           { LLGL_DBG_LABEL(desc) }
{
}

DbgRenderPass::DbgRenderPass(const RenderPass& instance, const RenderPassDescriptor& desc) :
    instance        { instance             },
    mutableInstance { nullptr              },
    desc            { desc                 },
    label           { LLGL_DBG_LABEL(desc) }
{
}

void DbgRenderPass::SetDebugName(const char* name)
{
    /* Render passes have to be named manually with an explicitly multable instance, because they can be queried from RenderTarget::GetRenderPass() */
    if (mutableInstance != nullptr)
    {
        /* Set or clear label */
        if (name != nullptr)
            label = name;
        else
            label.clear();

        /* Forward call to instance */
        mutableInstance->SetDebugName(name);
    }
}

std::uint32_t DbgRenderPass::NumEnabledColorAttachments() const
{
    return LLGL::NumEnabledColorAttachments(desc);
}

bool DbgRenderPass::AnySwapChainAttachmentsLoaded(const DbgSwapChain& swapChain) const
{
    const Format colorFormat = swapChain.GetColorFormat();
    if (IsColorFormat(colorFormat))
    {
        if (desc.colorAttachments[0].loadOp == AttachmentLoadOp::Load)
            return true;
    }
    const Format depthStencilFormat = swapChain.GetDepthStencilFormat();
    if (IsDepthFormat(depthStencilFormat))
    {
        if (desc.depthAttachment.loadOp == AttachmentLoadOp::Load)
            return true;
    }
    if (IsStencilFormat(depthStencilFormat))
    {
        if (desc.stencilAttachment.loadOp == AttachmentLoadOp::Load)
            return true;
    }
    return false;
}


} // /namespace LLGL



// ================================================================================
