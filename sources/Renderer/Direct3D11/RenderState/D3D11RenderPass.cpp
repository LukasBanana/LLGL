/*
 * D3D11RenderPass.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D11RenderPass.h"
#include "../../RenderPassUtils.h"
#include "../../../Core/Exception.h"
#include <LLGL/Utils/ForRange.h>


namespace LLGL
{


// Returns true if the color attachment is meant to be resolved; ResolveSubresource() only averages non-integer formats.
static bool IsD3D11ColorResolveEnabled(const AttachmentFormatDescriptor& desc, std::uint32_t samples)
{
    if (desc.resolveMode != ResolveMode::Disabled)
    {
        if (samples > 1 && desc.format != Format::Undefined)
        {
            if (desc.resolveMode == ResolveMode::Min || desc.resolveMode == ResolveMode::Max)
                LLGL_TRAP_FEATURE_NOT_SUPPORTED("Direct3D 11 does not support Min/Max resolve modes");
            if (desc.resolveMode == ResolveMode::Average && IsIntegerFormat(desc.format))
                LLGL_TRAP_FEATURE_NOT_SUPPORTED("Direct3D 11 supports Average resolve mode only for non-integer color formats");
        }
        return true;
    }
    return false;
}

D3D11RenderPass::D3D11RenderPass(const RenderPassDescriptor& desc)
{
    /* Determine which color attachments must be resolved at the end of a render pass */
    for_range(i, LLGL_MAX_NUM_COLOR_ATTACHMENTS)
    {
        if (IsD3D11ColorResolveEnabled(desc.colorAttachments[i], desc.samples))
            resolveColorMask_ |= (1u << i);
    }

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
