/*
 * RenderTargetUtils.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_RENDER_TARGET_UTILS_H
#define LLGL_RENDER_TARGET_UTILS_H


#include <LLGL/Export.h>
#include <LLGL/Format.h>
#include <cstdint>


namespace LLGL
{


struct RenderingLimits;
struct AttachmentDescriptor;
struct RenderTargetDescriptor;

/* ----- Functions ----- */

// Returns true if the specified render-target attachment is enabled, i.e. either 'texture' or 'format' is valid.
LLGL_EXPORT bool IsAttachmentEnabled(const AttachmentDescriptor& attachmentDesc);

// Returns the format of the specified render-target attachment.
LLGL_EXPORT Format GetAttachmentFormat(const AttachmentDescriptor& attachmentDesc);

/*
Returns the number of active color attachments in the specified render target descriptor.
The first inactive attachment breaks the count.
*/
LLGL_EXPORT std::uint32_t NumActiveColorAttachments(const RenderTargetDescriptor& renderTargetDesc);

/*
Returns the number of active resolve attachments in the specified render target descriptor.
The first inactive color attachment (not resolve attachment) breaks the count.
*/
LLGL_EXPORT std::uint32_t NumActiveResolveAttachments(const RenderTargetDescriptor& renderTargetDesc);

// Returns true if the specified render-target descriptor has any active attachments
LLGL_EXPORT bool HasAnyActiveAttachments(const RenderTargetDescriptor& desc);

// Returns the number of samples for the specified render-target descriptor and rendering limitations.
LLGL_EXPORT std::uint32_t GetLimitedRenderTargetSamples(const RenderingLimits& limits, const RenderTargetDescriptor& desc);


} // /namespace LLGL


#endif



// ================================================================================
