/*
 * NullRenderTarget.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "NullRenderTarget.h"
#include "NullTexture.h"
#include "../../CheckedCast.h"
#include "../../RenderTargetUtils.h"
#include "../../../Core/CoreUtils.h"


namespace LLGL
{


NullRenderTarget::NullRenderTarget(const RenderTargetDescriptor& desc) :
    desc { desc }
{
    BuildAttachmentArray();
    if (desc.debugName != nullptr)
        SetDebugName(desc.debugName);
}

void NullRenderTarget::SetDebugName(const char* name)
{
    if (name != nullptr)
        label_ = name;
    else
        label_.clear();
}

Extent2D NullRenderTarget::GetResolution() const
{
    return desc.resolution;
}

std::uint32_t NullRenderTarget::GetSamples() const
{
    return desc.samples;
}

std::uint32_t NullRenderTarget::GetNumColorAttachments() const
{
    return static_cast<std::uint32_t>(colorAttachments_.size());
}

bool NullRenderTarget::HasDepthAttachment() const
{
    return IsDepthFormat(depthStencilFormat_);
}

bool NullRenderTarget::HasStencilAttachment() const
{
    return IsStencilFormat(depthStencilFormat_);
}

const RenderPass* NullRenderTarget::GetRenderPass() const
{
    return nullptr; // TODO
}


/*
 * ======= Private: =======
 */

void NullRenderTarget::BuildAttachmentArray()
{
    /* Cache color attachments */
    for (const auto& attachment : desc.colorAttachments)
    {
        if (IsAttachmentEnabled(attachment))
        {
            if (auto texture = attachment.texture)
            {
                auto textureNull = LLGL_CAST(NullTexture*, texture);
                colorAttachments_.push_back(textureNull);
            }
            else
                colorAttachments_.push_back(MakeIntermediateAttachment(attachment.format, desc.samples));
        }
    }

    /* Cache resolve attachments */
    for (const auto& attachment : desc.resolveAttachments)
    {
        if (IsAttachmentEnabled(attachment))
        {
            if (auto texture = attachment.texture)
            {
                auto textureNull = LLGL_CAST(NullTexture*, texture);
                resolveAttachments_.push_back(textureNull);
            }
            else
                resolveAttachments_.push_back(MakeIntermediateAttachment(attachment.format));
        }
    }

    /* Cache depth-stencil attachment */
    if (IsAttachmentEnabled(desc.depthStencilAttachment))
    {
        if (auto* texture = desc.depthStencilAttachment.texture)
        {
            auto* textureNull = LLGL_CAST(NullTexture*, texture);
            depthStencilAttachment_ = textureNull;
            depthStencilFormat_     = textureNull->desc.format;
        }
        else
            depthStencilFormat_ = desc.depthStencilAttachment.format;
    }
}

NullTexture* NullRenderTarget::MakeIntermediateAttachment(const Format format, std::uint32_t samples)
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
    intermediateAttachments_.push_back(MakeUnique<NullTexture>(textureDesc));
    return intermediateAttachments_.back().get();
}


} // /namespace LLGL



// ================================================================================
