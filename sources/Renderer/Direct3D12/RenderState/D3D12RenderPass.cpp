/*
 * D3D12RenderPass.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12RenderPass.h"
#include "../../DescriptorHelper.h"
#include <LLGL/RenderPassFlags.h>


namespace LLGL
{


D3D12RenderPass::D3D12RenderPass(const RenderPassDescriptor& desc) :
    numColorAttachments_ { static_cast<UINT>(desc.colorAttachments.size()) }
{
    /* Check which color attachment must be cleared */
    FillClearColorAttachmentIndices(clearColorAttachments_, desc);

    /* Check if depth attachment must be cleared */
    if (desc.depthAttachment.loadOp == AttachmentLoadOp::Clear)
        clearFlagsDSV_ |= D3D12_CLEAR_FLAG_DEPTH;

    /* Check if stencil attachment must be cleared */
    if (desc.stencilAttachment.loadOp == AttachmentLoadOp::Clear)
        clearFlagsDSV_ |= D3D12_CLEAR_FLAG_STENCIL;
}


} // /namespace LLGL



// ================================================================================
