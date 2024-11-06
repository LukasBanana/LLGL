/*
 * D3D11RenderPass.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D11RenderPass.h"
#include "../../RenderPassUtils.h"


namespace LLGL
{


D3D11RenderPass::D3D11RenderPass(const RenderPassDescriptor& desc)
{
    /* Check which color attachment must be cleared */
    FillClearColorAttachmentIndices(LLGL_MAX_NUM_COLOR_ATTACHMENTS, clearColorAttachments_, desc);

    /* Check if depth attachment must be cleared */
    if (desc.depthAttachment.loadOp == AttachmentLoadOp::Clear)
        clearFlagsDSV_ |= D3D11_CLEAR_DEPTH;

    /* Check if stencil attachment must be cleared */
    if (desc.stencilAttachment.loadOp == AttachmentLoadOp::Clear)
        clearFlagsDSV_ |= D3D11_CLEAR_STENCIL;

    /* If we don't care about storing depth/stencil values, append flag to only read those values */
    if (desc.depthAttachment.storeOp == AttachmentStoreOp::Undefined && IsDepthFormat(desc.depthAttachment.format))
        attachmentFlagsDSV_ |= D3D11_DSV_READ_ONLY_DEPTH;
    if (desc.stencilAttachment.storeOp == AttachmentStoreOp::Undefined && IsStencilFormat(desc.stencilAttachment.format))
        attachmentFlagsDSV_ |= D3D11_DSV_READ_ONLY_STENCIL;
}


} // /namespace LLGL



// ================================================================================
