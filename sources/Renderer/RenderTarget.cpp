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


RenderTarget::~RenderTarget()
{
}

#if 1 // DEPRECATED

void RenderTarget::AttachDepthBuffer(const Extent2D& size) { /* dummy */ }
void RenderTarget::AttachStencilBuffer(const Extent2D& size) { /* dummy */ }
void RenderTarget::AttachDepthStencilBuffer(const Extent2D& size) { /* dummy */ }
void RenderTarget::AttachTexture(Texture& texture, const RenderTargetAttachmentDescriptor& attachmentDesc) { /* dummy */ }
void RenderTarget::DetachAll() { /* dummy */ }

#endif // /DEPRECATED


/*
 * ======= Protected: =======
 */

static std::string Extent2DToString(const Extent2D& res)
{
    return std::to_string(res.width) + 'x' + std::to_string(res.height);
}

void RenderTarget::ApplyResolution(const Extent2D& resolution)
{
    /* Validate texture attachment size */
    if (resolution.width == 0 || resolution.height == 0)
        throw std::invalid_argument("attachment to render target failed, due to invalid size of zero");

    /* Check if size matches the current resolution */
    if (resolution_ != Extent2D{ 0, 0 })
    {
        if (resolution != resolution_)
        {
            throw std::invalid_argument(
                "attachment to render target failed, due to resolution mismatch (" +
                Extent2DToString(resolution) + " given, but expected " + Extent2DToString(resolution_) + ")"
            );
        }
    }
    else
        resolution_ = resolution;
}

void RenderTarget::ApplyMipResolution(Texture& texture, std::uint32_t mipLevel)
{
    /* Apply texture size to render target resolution */
    auto size = texture.QueryMipLevelSize(mipLevel);
    ApplyResolution({ size.width, size.height });
}

void RenderTarget::ResetResolution()
{
    resolution_.width   = 0;
    resolution_.height  = 0;
}


} // /namespace LLGL



// ================================================================================
