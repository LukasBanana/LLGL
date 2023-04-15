/*
 * RenderPassUtils.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_RENDER_PASS_UTILS_H
#define LLGL_RENDER_PASS_UTILS_H


#include <LLGL/Export.h>
#include <LLGL/RenderPassFlags.h>
#include <cstdint>
#include <cstddef>


namespace LLGL
{


/* ----- Functions ----- */

// Returns the number of enabled color attachments in the specified render pass.
LLGL_EXPORT std::uint32_t NumEnabledColorAttachments(const RenderPassDescriptor& renderPassDesc);

// Fills the array of indices with the invalid index of 0xFF.
LLGL_EXPORT void ResetClearColorAttachmentIndices(
    std::uint32_t   numClearIndices,
    std::uint8_t*   outClearIndices
);

// Fills the array of indices for the color attachments that are meant to be cleared. Unused indices are filled with 0xFF.
LLGL_EXPORT std::uint32_t FillClearColorAttachmentIndices(
    std::uint32_t               numClearIndices,
    std::uint8_t*               outClearIndices,
    const RenderPassDescriptor& renderPassDesc
);


} // /namespace LLGL


#endif



// ================================================================================
