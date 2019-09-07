/*
 * GLRenderPass.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLRenderPass.h"
#include "../../DescriptorHelper.h"
#include <LLGL/CommandBufferFlags.h>


namespace LLGL
{


GLRenderPass::GLRenderPass(const RenderPassDescriptor& desc) :
    numColorAttachments_ { static_cast<std::uint8_t>(desc.colorAttachments.size()) }
{
    /* Check which color attachment must be cleared */
    if (FillClearColorAttachmentIndices(LLGL_MAX_NUM_COLOR_ATTACHMENTS, clearColorAttachments_, desc) > 0)
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
