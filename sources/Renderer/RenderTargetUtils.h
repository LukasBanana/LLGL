/*
 * RenderTargetUtils.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_RENDER_TARGET_UTILS_H
#define LLGL_RENDER_TARGET_UTILS_H


#include <LLGL/Texture.h>
#include <LLGL/RenderTargetFlags.h>


namespace LLGL
{


/* ----- Functions ----- */

// Returns true if the specified render-target attachment is enabled, i.e. either 'texture' or 'format' is valid.
inline bool IsAttachmentEnabled(const AttachmentDescriptor& attachmentDesc)
{
    return (attachmentDesc.texture != nullptr || attachmentDesc.format != Format::Undefined);
}

// Returns the format of the specified render-target attachment.
inline Format GetAttachmentFormat(const AttachmentDescriptor& attachmentDesc)
{
    if (attachmentDesc.format != Format::Undefined)
        return attachmentDesc.format;
    if (auto texture = attachmentDesc.texture)
        return texture->GetFormat();
    return Format::Undefined;
}

/*
Returns the number of active color attachments in the specified render target descriptor.
The first inactive attachment breaks the count.
*/
inline std::uint32_t NumActiveColorAttachments(const RenderTargetDescriptor& renderTargetDesc)
{
    std::uint32_t n = 0;
    while (n < LLGL_MAX_NUM_COLOR_ATTACHMENTS && IsAttachmentEnabled(renderTargetDesc.colorAttachments[n]))
        ++n;
    return n;
}

/*
Returns the number of active resolve attachments in the specified render target descriptor.
The first inactive color attachment (not resolve attachment) breaks the count.
*/
inline std::uint32_t NumActiveResolveAttachments(const RenderTargetDescriptor& renderTargetDesc)
{
    std::uint32_t n = 0;
    for (std::uint32_t i = 0; i < LLGL_MAX_NUM_COLOR_ATTACHMENTS && IsAttachmentEnabled(renderTargetDesc.colorAttachments[i]); ++i)
    {
        if (renderTargetDesc.resolveAttachments[i].texture != nullptr)
            ++n;
    }
    return n;
}


} // /namespace LLGL


#endif



// ================================================================================
