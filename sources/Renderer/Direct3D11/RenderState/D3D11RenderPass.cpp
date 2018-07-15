/*
 * D3D11RenderPass.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11RenderPass.h"
#include <LLGL/RenderPassFlags.h>


namespace LLGL
{


D3D11RenderPass::D3D11RenderPass(const RenderPassDescriptor& desc)
{
    /* Check which color attachment must be cleared */
    std::size_t     i           = 0;
    std::uint8_t    bufferIndex = 0;

    for (const auto& attachment : desc.colorAttachments)
    {
        if (attachment.loadOp == AttachmentLoadOp::Clear)
            clearColorAttachments_[i++] = bufferIndex;
        ++bufferIndex;
    }

    /* Initialize remaining attachment indices */
    for (; i < LLGL_MAX_NUM_COLOR_ATTACHMENTS; ++i)
        clearColorAttachments_[i] = 0xFF;

    /* Check if depth attachment must be cleared */
    if (desc.depthAttachment.loadOp == AttachmentLoadOp::Clear)
        clearFlagsDSV_ |= D3D11_CLEAR_DEPTH;

    /* Check if stencil attachment must be cleared */
    if (desc.stencilAttachment.loadOp == AttachmentLoadOp::Clear)
        clearFlagsDSV_ |= D3D11_CLEAR_STENCIL;
}


} // /namespace LLGL



// ================================================================================
