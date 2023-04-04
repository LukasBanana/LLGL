/*
 * GLRenderPass.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "GLRenderPass.h"
#include "../../RenderPassUtils.h"
#include <LLGL/CommandBufferFlags.h>


namespace LLGL
{


GLRenderPass::GLRenderPass(const RenderPassDescriptor& desc)
{
    /* Check which color attachment must be cleared */
    numColorAttachments_ = static_cast<std::uint8_t>(FillClearColorAttachmentIndices(LLGL_MAX_NUM_COLOR_ATTACHMENTS, clearColorAttachments_, desc));
    if (numColorAttachments_ > 0)
        clearMask_ |= GL_COLOR_BUFFER_BIT;

    /* Check if depth attachment must be cleared */
    if (desc.depthAttachment.loadOp == AttachmentLoadOp::Clear)
        clearMask_ |= GL_DEPTH_BUFFER_BIT;

    /* Check if stencil attachment must be cleared */
    if (desc.stencilAttachment.loadOp == AttachmentLoadOp::Clear)
        clearMask_ |= GL_STENCIL_BUFFER_BIT;
}


} // /namespace LLGL



// ================================================================================
