/*
 * RenderPassUtils.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "RenderPassUtils.h"
#include <LLGL/Utils/ForRange.h>
#include "../Core/Assertion.h"


namespace LLGL
{


LLGL_EXPORT std::uint32_t NumEnabledColorAttachments(const RenderPassDescriptor& renderPassDesc)
{
    std::uint32_t n = 0;
    while (n < LLGL_MAX_NUM_COLOR_ATTACHMENTS && renderPassDesc.colorAttachments[n].format != Format::Undefined)
        ++n;
    return n;
}

LLGL_EXPORT void ResetClearColorAttachmentIndices(
    std::uint32_t   numClearIndices,
    std::uint8_t*   outClearIndices)
{
    for_range(i, numClearIndices)
        outClearIndices[i] = 0xFF;
}

LLGL_EXPORT std::uint32_t FillClearColorAttachmentIndices(
    std::uint32_t               numClearIndices,
    std::uint8_t*               outClearIndices,
    const RenderPassDescriptor& renderPassDesc)
{
    LLGL_ASSERT(numClearIndices <= LLGL_MAX_NUM_COLOR_ATTACHMENTS);

    /* Check which color attachment must be cleared */
    std::uint32_t clearIndex = 0;

    for_range(bufferIndex, LLGL_MAX_NUM_COLOR_ATTACHMENTS)
    {
        const auto& colorAttachment = renderPassDesc.colorAttachments[bufferIndex];
        if (colorAttachment.format != Format::Undefined && clearIndex < numClearIndices)
        {
            if (colorAttachment.loadOp == AttachmentLoadOp::Clear)
                outClearIndices[clearIndex++] = bufferIndex;
        }
        else
            break;
    }

    /* Initialize remaining attachment indices */
    const auto numColorAttachmentsToClear = clearIndex;

    for (; clearIndex < numClearIndices; ++clearIndex)
        outClearIndices[clearIndex] = 0xFF;

    return numColorAttachmentsToClear;
}


} // /namespace LLGL



// ================================================================================
