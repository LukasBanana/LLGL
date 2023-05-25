/*
 * RenderTarget.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/RenderTarget.h>
#include <LLGL/Texture.h>
#include "../Core/Exception.h"


namespace LLGL
{


/*
 * ======= Protected: =======
 */

void RenderTarget::ValidateResolution(const Extent2D& attachmentResolution)
{
    /* Validate texture attachment size */
    if (attachmentResolution.width == 0 || attachmentResolution.height == 0)
        LLGL_TRAP("invalid resolution of render tartget attachment: %ux%u", attachmentResolution.width, attachmentResolution.height);

    /* Check if size matches the current resolution */
    const Extent2D targetResolution = GetResolution();
    if (targetResolution != attachmentResolution)
    {
        LLGL_TRAP(
            "resolution mismatch of render target attachment: %ux%u is specified, but expected %ux%u",
            attachmentResolution.width, attachmentResolution.height,
            targetResolution.width, targetResolution.height
        );
    }
}

void RenderTarget::ValidateMipResolution(const Texture& texture, std::uint32_t mipLevel)
{
    /* Apply texture size to render target resolution */
    auto size = texture.GetMipExtent(mipLevel);
    ValidateResolution({ size.width, size.height });
}


} // /namespace LLGL



// ================================================================================
