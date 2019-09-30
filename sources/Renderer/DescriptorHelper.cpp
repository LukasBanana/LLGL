/*
 * DescriptorHelper.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "DescriptorHelper.h"
#include <LLGL/StaticLimits.h>
#include <LLGL/RenderPassFlags.h>
#include <algorithm>


namespace LLGL
{


LLGL_EXPORT void ResetClearColorAttachmentIndices(
    std::size_t     numColorAttachments,
    std::uint8_t*   colorAttachmentsIndices)
{
    for (std::size_t i = 0; i < numColorAttachments; ++i)
        colorAttachmentsIndices[i] = 0xFF;
}

LLGL_EXPORT std::uint8_t FillClearColorAttachmentIndices(
    std::size_t                 numColorAttachments,
    std::uint8_t*               colorAttachmentsIndices,
    const RenderPassDescriptor& renderPassDesc)
{
    /* Check which color attachment must be cleared */
    std::size_t i = 0;

    for (std::uint8_t bufferIndex = 0; i < numColorAttachments && i < renderPassDesc.colorAttachments.size(); ++bufferIndex)
    {
        if (renderPassDesc.colorAttachments[bufferIndex].loadOp == AttachmentLoadOp::Clear)
            colorAttachmentsIndices[i++] = bufferIndex;
    }

    /* Initialize remaining attachment indices */
    const auto n = i;

    while (i < numColorAttachments)
        colorAttachmentsIndices[i++] = 0xFF;

    return static_cast<std::uint8_t>(n);
}


} // /namespace LLGL



// ================================================================================
