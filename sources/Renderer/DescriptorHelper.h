/*
 * DescriptorHelper.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_DESCRIPTOR_HELPER_H
#define LLGL_DESCRIPTOR_HELPER_H


#include <LLGL/Export.h>
#include <cstdint>
#include <cstddef>


namespace LLGL
{

//TODO: merge this header and source with "TextureUtils.cpp/h"

struct RenderPassDescriptor;

/* ----- Functions ----- */

// Fills the array of indices with the invalid index of 0xFF.
LLGL_EXPORT void ResetClearColorAttachmentIndices(
    std::size_t     numColorAttachments,
    std::uint8_t*   colorAttachmentsIndices
);

// Fills the array of indices for the color attachments that are meant to be cleared.
LLGL_EXPORT std::uint8_t FillClearColorAttachmentIndices(
    std::size_t                 numColorAttachments,
    std::uint8_t*               colorAttachmentsIndices,
    const RenderPassDescriptor& renderPassDesc
);


} // /namespace LLGL


#endif



// ================================================================================
