/*
 * D3D12RenderPass.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "D3D12RenderPass.h"
#include "../D3D12Types.h"
#include "../Texture/D3D12Texture.h"
#include "../../DescriptorHelper.h"
#include "../../CheckedCast.h"
#include <LLGL/RenderPassFlags.h>
#include <LLGL/RenderTargetFlags.h>
#include <algorithm>


namespace LLGL
{


D3D12RenderPass::D3D12RenderPass(const RenderPassDescriptor& desc)
{
    BuildAttachments(desc);
}

void D3D12RenderPass::BuildAttachments(const RenderPassDescriptor& desc)
{
    /* Reset flags and depth-stencil format */
    clearFlagsDSV_          = 0;
    numColorAttachments_    = std::min(LLGL_MAX_NUM_COLOR_ATTACHMENTS, static_cast<UINT>(desc.colorAttachments.size()));

    SetDSVFormat(DXGI_FORMAT_UNKNOWN);

    /* Check which color attachment must be cleared */
    FillClearColorAttachmentIndices(LLGL_MAX_NUM_COLOR_ATTACHMENTS, clearColorAttachments_, desc);

    /* Check if depth attachment must be cleared */
    if (desc.depthAttachment.loadOp == AttachmentLoadOp::Clear)
        clearFlagsDSV_ |= D3D12_CLEAR_FLAG_DEPTH;

    /* Check if stencil attachment must be cleared */
    if (desc.stencilAttachment.loadOp == AttachmentLoadOp::Clear)
        clearFlagsDSV_ |= D3D12_CLEAR_FLAG_STENCIL;

    /* Store native color formats */
    for (UINT i = 0; i < LLGL_MAX_NUM_COLOR_ATTACHMENTS; ++i)
    {
        if (i < numColorAttachments_)
            SetRTVFormat(i, D3D12Types::Map(desc.colorAttachments[i].format));
        else
            SetRTVFormat(i, DXGI_FORMAT_UNKNOWN);
    }

    /* Store native depth-stencil format */
    if (desc.depthAttachment.format != desc.stencilAttachment.format &&
        desc.depthAttachment.format != Format::Undefined             &&
        desc.stencilAttachment.format != Format::Undefined)
    {
        throw std::invalid_argument("mismatch between depth and stencil attachment formats");
    }

    if (desc.depthAttachment.format != Format::Undefined)
        SetDSVFormat(D3D12Types::Map(desc.depthAttachment.format));
    else if (desc.stencilAttachment.format != Format::Undefined)
        SetDSVFormat(D3D12Types::Map(desc.stencilAttachment.format));
}

void D3D12RenderPass::BuildAttachments(
    std::size_t                 numAttachmentDescs,
    const AttachmentDescriptor* attachmentDescs,
    const DXGI_FORMAT           defaultDepthStencilFormat)
{
    /* Reset clear flags and depth-stencil format */
    clearFlagsDSV_ = 0;
    SetDSVFormat(DXGI_FORMAT_UNKNOWN);
    ResetClearColorAttachmentIndices(LLGL_MAX_NUM_COLOR_ATTACHMENTS, clearColorAttachments_);

    /* Check which color attachment must be cleared */
    std::uint8_t colorAttachment = 0;

    for (UINT i = 0; i < numAttachmentDescs; ++i)
    {
        const auto& attachment = attachmentDescs[i];

        if (auto texture = attachment.texture)
        {
            auto textureD3D = LLGL_CAST(D3D12Texture*, texture);
            if (attachment.type == AttachmentType::Color)
            {
                if (colorAttachment < LLGL_MAX_NUM_COLOR_ATTACHMENTS)
                {
                    /* Store texture color format and attachment index */
                    SetRTVFormat(colorAttachment++, textureD3D->GetFormat());
                }
            }
            else
            {
                /* Use texture depth-stencil format */
                SetDSVFormat(textureD3D->GetFormat());
            }
        }
        else if (attachment.type != AttachmentType::Color)
        {
            /* Use default depth-stencil format */
            SetDSVFormat(defaultDepthStencilFormat);
        }
    }

    numColorAttachments_ = colorAttachment;

    /* Reset remaining color formats */
    for (UINT i = numColorAttachments_; i < LLGL_MAX_NUM_COLOR_ATTACHMENTS; ++i)
        SetRTVFormat(i, DXGI_FORMAT_UNKNOWN);
}


/*
 * ======= Private: =======
 */

void D3D12RenderPass::SetDSVFormat(DXGI_FORMAT format)
{
    dsvFormat_ = D3D12Types::ToDXGIFormatDSV(format);
}

void D3D12RenderPass::SetRTVFormat(UINT colorAttachment, DXGI_FORMAT format)
{
    rtvFormats_[colorAttachment] = format;
}


} // /namespace LLGL



// ================================================================================
