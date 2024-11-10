/*
 * D3D12RenderPass.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D12RenderPass.h"
#include "../D3D12Device.h"
#include "../D3D12Types.h"
#include "../Texture/D3D12Texture.h"
#include "../../RenderPassUtils.h"
#include "../../RenderTargetUtils.h"
#include "../../CheckedCast.h"
#include "../../TextureUtils.h"
#include "../../../Core/Exception.h"
#include <LLGL/RenderTargetFlags.h>
#include <LLGL/Utils/ForRange.h>
#include <algorithm>


namespace LLGL
{


D3D12RenderPass::D3D12RenderPass(
    const D3D12Device& device,
    const RenderPassDescriptor& desc)
{
    BuildAttachments(device, desc);
}

void D3D12RenderPass::BuildAttachments(
    const D3D12Device&          device,
    const RenderPassDescriptor& desc)
{
    /* Check which color attachment must be cleared */
    numColorAttachments_    = NumEnabledColorAttachments(desc);
    clearFlagsDSV_          = 0;

    FillClearColorAttachmentIndices(LLGL_MAX_NUM_COLOR_ATTACHMENTS, clearColorAttachments_, desc);

    /* Check if depth attachment must be cleared */
    if (desc.depthAttachment.loadOp == AttachmentLoadOp::Clear)
        clearFlagsDSV_ |= D3D12_CLEAR_FLAG_DEPTH;

    /* Check if stencil attachment must be cleared */
    if (desc.stencilAttachment.loadOp == AttachmentLoadOp::Clear)
        clearFlagsDSV_ |= D3D12_CLEAR_FLAG_STENCIL;

    /* Store native color formats */
    for_range(i, numColorAttachments_)
        SetRTVFormat(DXTypes::ToDXGIFormat(desc.colorAttachments[i].format), i);
    for_subrange(i, numColorAttachments_, LLGL_MAX_NUM_COLOR_ATTACHMENTS)
        SetRTVFormat(DXGI_FORMAT_UNKNOWN, i);

    /* Store native depth-stencil format */
    if (desc.depthAttachment.format   != desc.stencilAttachment.format &&
        desc.depthAttachment.format   != Format::Undefined             &&
        desc.stencilAttachment.format != Format::Undefined)
    {
        LLGL_TRAP("mismatch between depth and stencil attachment formats");
    }

    if (desc.depthAttachment.format != Format::Undefined)
        SetDSVFormat(DXTypes::ToDXGIFormat(desc.depthAttachment.format));
    else if (desc.stencilAttachment.format != Format::Undefined)
        SetDSVFormat(DXTypes::ToDXGIFormat(desc.stencilAttachment.format));
    else
        SetDSVFormat(DXGI_FORMAT_UNKNOWN);

    /* If we don't care about storing depth/stencil values, append flag to only read those values */
    if (desc.depthAttachment.storeOp == AttachmentStoreOp::Undefined && IsDepthFormat(desc.depthAttachment.format))
        attachmentFlagsDSV_ |= D3D12_DSV_FLAG_READ_ONLY_DEPTH;
    if (desc.stencilAttachment.storeOp == AttachmentStoreOp::Undefined && IsStencilFormat(desc.stencilAttachment.format))
        attachmentFlagsDSV_ |= D3D12_DSV_FLAG_READ_ONLY_STENCIL;

    /* Store sample descriptor */
    sampleDesc_ = device.FindSuitableSampleDesc(numColorAttachments_, rtvFormats_, GetClampedSamples(desc.samples));
}

void D3D12RenderPass::BuildAttachments(
    UINT                    numColorFormats,
    const DXGI_FORMAT*      colorFormats,
    const DXGI_FORMAT       depthStencilFormat,
    const DXGI_SAMPLE_DESC& sampleDesc)
{
    /* Reset clear flags */
    clearFlagsDSV_ = 0;
    ResetClearColorAttachmentIndices(LLGL_MAX_NUM_COLOR_ATTACHMENTS, clearColorAttachments_);

    /* Store color attachment formats */
    numColorAttachments_ = numColorFormats;
    for_range(i, numColorFormats)
        SetRTVFormat(colorFormats[i], i);

    /* Store depth-stencil attachment format */
    SetDSVFormat(depthStencilFormat);

    /* Store sample descriptor */
    sampleDesc_ = sampleDesc;
}


/*
 * ======= Private: =======
 */

void D3D12RenderPass::SetDSVFormat(DXGI_FORMAT format)
{
    dsvFormat_ = DXTypes::ToDXGIFormatDSV(format);
}

void D3D12RenderPass::SetRTVFormat(DXGI_FORMAT format, UINT colorAttachment)
{
    rtvFormats_[colorAttachment] = DXTypes::ToDXGIFormatRTV(format);
}


} // /namespace LLGL



// ================================================================================
