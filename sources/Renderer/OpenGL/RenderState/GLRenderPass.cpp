/*
 * GLRenderPass.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "GLRenderPass.h"
#include "../../RenderPassUtils.h"
#include "../../../Core/Exception.h"
#include <LLGL/CommandBufferFlags.h>
#include <LLGL/Utils/ForRange.h>


namespace LLGL
{


// Returns true if the attachment is meant to be resolved; Resolving via glBlitFramebuffer cannot select Min/Max or average integer/depth-stencil samples.
static bool IsGLResolveEnabled(const AttachmentFormatDescriptor& desc, std::uint32_t samples, bool isColorAttachment)
{
    if (desc.resolveMode != ResolveMode::Disabled)
    {
        if (samples > 1 && desc.format != Format::Undefined)
        {
            if (desc.resolveMode == ResolveMode::Min || desc.resolveMode == ResolveMode::Max)
                LLGL_TRAP_FEATURE_NOT_SUPPORTED("OpenGL does not support Min/Max resolve modes");
            if (desc.resolveMode == ResolveMode::Average && (!isColorAttachment || IsIntegerFormat(desc.format)))
                LLGL_TRAP_FEATURE_NOT_SUPPORTED("OpenGL supports Average resolve mode only for non-integer color formats");
        }
        return true;
    }
    return false;
}

GLRenderPass::GLRenderPass(const RenderPassDescriptor& desc)
{
    /* Determine which attachments must be resolved at the end of a render pass */
    for_range(i, LLGL_MAX_NUM_COLOR_ATTACHMENTS)
    {
        if (IsGLResolveEnabled(desc.colorAttachments[i], desc.samples, true))
            resolveColorMask_ |= (1u << i);
    }
    if (IsGLResolveEnabled(desc.depthAttachment, desc.samples, false))
        resolveDepthStencilMask_ |= GL_DEPTH_BUFFER_BIT;
    if (IsGLResolveEnabled(desc.stencilAttachment, desc.samples, false))
        resolveDepthStencilMask_ |= GL_STENCIL_BUFFER_BIT;

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
