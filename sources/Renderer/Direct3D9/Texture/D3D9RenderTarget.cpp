/*
 * D3D9RenderTarget.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D9RenderTarget.h"
#include "D3D9Texture.h"
#include "../../CheckedCast.h"
#include "../../RenderTargetUtils.h"
#include "../../../Core/CoreUtils.h"


namespace LLGL
{


D3D9RenderTarget::D3D9RenderTarget(IDirect3DDevice9* device, const RenderTargetDescriptor& desc) :
    desc { desc }
{
    BuildAttachmentArray(device);
    if (desc.debugName != nullptr)
        SetDebugName(desc.debugName);
}

void D3D9RenderTarget::SetDebugName(const char* name)
{
    if (name != nullptr)
        label_ = name;
    else
        label_.clear();
}

Extent2D D3D9RenderTarget::GetResolution() const
{
    return desc.resolution;
}

std::uint32_t D3D9RenderTarget::GetSamples() const
{
    return desc.samples;
}

std::uint32_t D3D9RenderTarget::GetNumColorAttachments() const
{
    return static_cast<std::uint32_t>(colorAttachments_.size());
}

bool D3D9RenderTarget::HasDepthAttachment() const
{
    return IsDepthFormat(depthStencilFormat_);
}

bool D3D9RenderTarget::HasStencilAttachment() const
{
    return IsStencilFormat(depthStencilFormat_);
}

const RenderPass* D3D9RenderTarget::GetRenderPass() const
{
    return nullptr; // TODO
}


/*
 * ======= Private: =======
 */

void D3D9RenderTarget::BuildAttachmentArray(IDirect3DDevice9* device)
{
    /* Cache color attachments */
    for (const auto& attachment : desc.colorAttachments)
    {
        if (IsAttachmentEnabled(attachment))
        {
            if (auto texture = attachment.texture)
            {
                auto textureD3D9 = LLGL_CAST(D3D9Texture*, texture);
                colorAttachments_.push_back(textureD3D9);
            }
            else
                colorAttachments_.push_back(MakeIntermediateAttachment(device, attachment.format, desc.samples));
        }
    }

    /* Cache resolve attachments */
    for (const auto& attachment : desc.resolveAttachments)
    {
        if (IsAttachmentEnabled(attachment))
        {
            if (auto texture = attachment.texture)
            {
                auto textureD3D9 = LLGL_CAST(D3D9Texture*, texture);
                resolveAttachments_.push_back(textureD3D9);
            }
            else
                resolveAttachments_.push_back(MakeIntermediateAttachment(device, attachment.format));
        }
    }

    /* Cache depth-stencil attachment */
    if (IsAttachmentEnabled(desc.depthStencilAttachment))
    {
        if (auto* texture = desc.depthStencilAttachment.texture)
        {
            auto* textureD3D9 = LLGL_CAST(D3D9Texture*, texture);
            depthStencilAttachment_ = textureD3D9;
            depthStencilFormat_     = textureD3D9->GetFormat();
        }
        else
            depthStencilFormat_ = desc.depthStencilAttachment.format;
    }
}

D3D9Texture* D3D9RenderTarget::MakeIntermediateAttachment(IDirect3DDevice9* device, const Format format, std::uint32_t samples)
{
    TextureDescriptor textureDesc;
    {
        textureDesc.type            = (samples > 1 ? TextureType::Texture2DMS : TextureType::Texture2D);
        textureDesc.bindFlags       = BindFlags::ColorAttachment;
        textureDesc.miscFlags       = MiscFlags::FixedSamples;
        textureDesc.format          = format;
        textureDesc.extent.width    = desc.resolution.width;
        textureDesc.extent.height   = desc.resolution.height;
        textureDesc.mipLevels       = 1;
        textureDesc.samples         = samples;
    };
    intermediateAttachments_.push_back(MakeUnique<D3D9Texture>(device, textureDesc));
    return intermediateAttachments_.back().get();
}


} // /namespace LLGL



// ================================================================================
