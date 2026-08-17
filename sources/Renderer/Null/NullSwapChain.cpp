/*
 * NullSwapChain.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "NullSwapChain.h"


namespace LLGL
{


static Format ChooseColorFormat(const SwapChainDescriptor& desc)
{
    /* Honor an explicitly requested color format and ignore the deprecated 'colorBits' field otherwise */
    if (IsColorFormat(desc.colorFormat))
        return desc.colorFormat;
    return Format::RGBA8UNorm;
}

static Format ChooseDepthStencilFormat(const SwapChainDescriptor& desc)
{
    /* Honor an explicitly requested depth-stencil format */
    if (IsDepthOrStencilFormat(desc.depthStencilFormat))
        return desc.depthStencilFormat;

    /* Deduce the format from the deprecated depth/stencil bit sizes */
    if (desc.depthBits == 32)
    {
        if (desc.stencilBits != 0)
            return Format::D32FloatS8X24UInt;
        else
            return Format::D32Float;
    }
    else
    {
        if (desc.stencilBits != 0)
            return Format::D24UNormS8UInt;
        else
            return Format::D32Float;
    }
}

NullSwapChain::NullSwapChain(
    const SwapChainDescriptor&      desc,
    const std::shared_ptr<Surface>& surface,
    const RendererInfo&             rendererInfo)
:
    SwapChain           { desc                                                       },
    samples_            { desc.samples                                               },
    colorFormat_        { ChooseColorFormat(desc)                                    },
    depthStencilFormat_ { ChooseDepthStencilFormat(desc)                             }
{
    SetOrCreateSurface(surface, SwapChain::BuildDefaultSurfaceTitle(rendererInfo), desc);

    if (desc.debugName != nullptr)
        SetDebugName(desc.debugName);

    /* Show default surface */
    if (!surface)
        ShowSurface();
}

void NullSwapChain::SetDebugName(const char* name)
{
    if (name != nullptr)
        label_ = name;
    else
        label_.clear();
}

bool NullSwapChain::IsPresentable() const
{
    return true; // dummy
}

void NullSwapChain::Present()
{
    // dummy
}

std::uint32_t NullSwapChain::GetCurrentSwapIndex() const
{
    return 0; // dummy
}

std::uint32_t NullSwapChain::GetNumSwapBuffers() const
{
    return 1; // dummy
}

std::uint32_t NullSwapChain::GetSamples() const
{
    return samples_;
}

Format NullSwapChain::GetColorFormat() const
{
    return colorFormat_;
}

Format NullSwapChain::GetDepthStencilFormat() const
{
    return depthStencilFormat_;
}

bool NullSwapChain::SetVsyncInterval(std::uint32_t vsyncInterval)
{
    vsyncInterval_ = vsyncInterval;
    return true;
}

const RenderPass* NullSwapChain::GetRenderPass() const
{
    return renderPass_;
}

Extent2D NullSwapChain::ResizeBuffersPrimary(const Extent2D& resolution)
{
    return resolution;
}


} // /namespace LLGL



// ================================================================================
