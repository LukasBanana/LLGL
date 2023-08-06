/*
 * RenderTargetUtils.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "RenderTargetUtils.h"
#include <LLGL/RenderSystemFlags.h>
#include <LLGL/RenderTargetFlags.h>
#include <LLGL/Texture.h>
#include <algorithm>


namespace LLGL
{


/* ----- Functions ----- */

LLGL_EXPORT bool IsAttachmentEnabled(const AttachmentDescriptor& attachmentDesc)
{
    return (attachmentDesc.texture != nullptr || attachmentDesc.format != Format::Undefined);
}

LLGL_EXPORT Format GetAttachmentFormat(const AttachmentDescriptor& attachmentDesc)
{
    if (attachmentDesc.format != Format::Undefined)
        return attachmentDesc.format;
    if (auto texture = attachmentDesc.texture)
        return texture->GetFormat();
    return Format::Undefined;
}

LLGL_EXPORT std::uint32_t NumActiveColorAttachments(const RenderTargetDescriptor& renderTargetDesc)
{
    std::uint32_t n = 0;
    while (n < LLGL_MAX_NUM_COLOR_ATTACHMENTS && IsAttachmentEnabled(renderTargetDesc.colorAttachments[n]))
        ++n;
    return n;
}

LLGL_EXPORT std::uint32_t NumActiveResolveAttachments(const RenderTargetDescriptor& renderTargetDesc)
{
    std::uint32_t n = 0;
    for (std::uint32_t i = 0; i < LLGL_MAX_NUM_COLOR_ATTACHMENTS && IsAttachmentEnabled(renderTargetDesc.colorAttachments[i]); ++i)
    {
        if (renderTargetDesc.resolveAttachments[i].texture != nullptr)
            ++n;
    }
    return n;
}

LLGL_EXPORT bool HasAnyActiveAttachments(const RenderTargetDescriptor& desc)
{
    return (NumActiveColorAttachments(desc) > 0 || IsAttachmentEnabled(desc.depthStencilAttachment));
}

LLGL_EXPORT std::uint32_t GetLimitedRenderTargetSamples(const RenderingLimits& limits, const RenderTargetDescriptor& desc)
{
    if (desc.samples > 0)
    {
        if (HasAnyActiveAttachments(desc))
        {
            const Format        depthStencilFormat      = GetAttachmentFormat(desc.depthStencilAttachment);
            const std::uint32_t maxColorBufferSamples   = (IsAttachmentEnabled(desc.colorAttachments[0]) ? limits.maxColorBufferSamples : desc.samples);
            const std::uint32_t maxDepthBufferSamples   = (IsDepthFormat(depthStencilFormat) ? limits.maxDepthBufferSamples : desc.samples);
            const std::uint32_t maxStencilBufferSamples = (IsStencilFormat(depthStencilFormat) ? limits.maxStencilBufferSamples : desc.samples);
            return std::min({ desc.samples, maxColorBufferSamples, maxDepthBufferSamples, maxStencilBufferSamples });
        }
        return std::min(desc.samples, limits.maxNoAttachmentSamples);
    }
    return 0u;
}


} // /namespace LLGL



// ================================================================================
