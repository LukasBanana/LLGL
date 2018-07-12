/*
 * GLRenderPass.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLRenderPass.h"
#include <LLGL/CommandBufferFlags.h>


namespace LLGL
{


GLRenderPass::GLRenderPass(const RenderPassDescriptor& desc)
{
    /* Check which color attachment must be cleared */
    std::size_t     i           = 0;
    std::uint8_t    bufferIndex = 0;

    for (const auto& attachment : desc.colorAttachments)
    {
        if (attachment.loadOp == AttachmentLoadOp::Clear)
        {
            clearColorAttachments_[i++] = bufferIndex;
            clearMask_ |= GL_COLOR_BUFFER_BIT;
        }
        ++bufferIndex;
    }

    /* Initialize remaining attachment indices */
    for (; i < LLGL_MAX_NUM_COLOR_ATTACHMENTS; ++i)
        clearColorAttachments_[i] = 0xFF;

    /* Check if depth attachment must be cleared */
    if (desc.depthAttachment.loadOp == AttachmentLoadOp::Clear)
        clearMask_ |= GL_DEPTH_BUFFER_BIT;

    /* Check if stencil attachment must be cleared */
    if (desc.stencilAttachment.loadOp == AttachmentLoadOp::Clear)
        clearMask_ |= GL_STENCIL_BUFFER_BIT;
}


} // /namespace LLGL



// ================================================================================
