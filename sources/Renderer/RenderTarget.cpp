/*
 * RenderTarget.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
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

RenderTarget::RenderTarget(const Extent2D& resolution) :
    resolution_ { resolution }
{
    if (resolution_.width == 0 || resolution_.height == 0)
        throw std::invalid_argument("invalid resolution of render target (at least one component is zero)");
}

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
    if (resolution_ != resolution)
    {
        throw std::invalid_argument(
            "resolution mismatch of render target attachment (" +
            Extent2DToString(resolution) + " is specified, but expected " + Extent2DToString(resolution_) + ")"
        );
    }
}

void RenderTarget::ValidateMipResolution(const Texture& texture, std::uint32_t mipLevel)
{
    /* Apply texture size to render target resolution */
    auto size = texture.QueryMipExtent(mipLevel);
    ValidateResolution({ size.width, size.height });
}


} // /namespace LLGL



// ================================================================================
