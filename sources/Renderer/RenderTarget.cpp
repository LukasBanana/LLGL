/*
 * RenderTarget.cpp
 * 
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/RenderTarget.h>
#include <LLGL/Texture.h>
#include <stdexcept>
#include <string>


namespace LLGL
{


/*
 * ======= Protected: =======
 */

static std::string Extent2DToString(const Extent2D& res)
{
    return (std::to_string(res.width) + 'x' + std::to_string(res.height));
}

void RenderTarget::ValidateResolution(const Extent2D& resolution)
{
    /* Validate texture attachment size */
    if (resolution.width == 0 || resolution.height == 0)
        throw std::invalid_argument("invalid resolution of render tartget attachment (at least one component is zero)");

    /* Check if size matches the current resolution */
    if (GetResolution() != resolution)
    {
        throw std::invalid_argument(
            "resolution mismatch of render target attachment (" +
            Extent2DToString(resolution) + " is specified, but expected " + Extent2DToString(GetResolution()) + ")"
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
