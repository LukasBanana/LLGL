/*
 * DescriptorHelper.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "DescriptorHelper.h"
#include "StaticLimits.h"
#include <LLGL/RenderPassFlags.h>


namespace LLGL
{


LLGL_EXPORT std::uint8_t FillClearColorAttachmentIndices(std::uint8_t* clearColorAttachments, const RenderPassDescriptor& renderPassDesc)
{
    /* Check which color attachment must be cleared */
    std::uint8_t i = 0, bufferIndex = 0;

    for (const auto& attachment : renderPassDesc.colorAttachments)
    {
        if (attachment.loadOp == AttachmentLoadOp::Clear)
            clearColorAttachments[i++] = bufferIndex;
        ++bufferIndex;
    }

    /* Initialize remaining attachment indices */
    const auto n = i;

    for (; i < LLGL_MAX_NUM_COLOR_ATTACHMENTS; ++i)
        clearColorAttachments[i] = 0xFF;

    return n;
}


} // /namespace LLGL



// ================================================================================
