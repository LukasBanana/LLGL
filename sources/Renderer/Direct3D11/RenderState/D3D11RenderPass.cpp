/*
 * D3D11RenderPass.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D11RenderPass.h"
#include "../../DescriptorHelper.h"
#include <LLGL/RenderPassFlags.h>


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
}


} // /namespace LLGL



// ================================================================================
