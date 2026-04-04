/*
 * WGSwapChain.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "WGSwapChain.h"
#include "WGRenderSystem.h"
#include "../../Core/Assertion.h"


namespace LLGL
{


WGSwapChain::WGSwapChain(const SwapChainDescriptor& desc, const std::shared_ptr<Surface>& surface, WGRenderSystem& renderSystem)
{
    /* Setup surface for the swap-chain */
    SetOrCreateSurface(surface, SwapChain::BuildDefaultSurfaceTitle(renderSystem.GetRendererInfo()), desc);

    //TODO
}

bool WGSwapChain::IsPresentable() const
{
    return true; // dummy
}

void WGSwapChain::Present()
{
    //wgpuSurfacePresent(surface_); //TODO
}

std::uint32_t WGSwapChain::GetCurrentSwapIndex() const
{
    return 0; //todo
}

std::uint32_t WGSwapChain::GetNumSwapBuffers() const
{
    return 1; //todo
}

std::uint32_t WGSwapChain::GetSamples() const
{
    return 1; //todo
}

Format WGSwapChain::GetColorFormat() const
{
    return Format::RGBA8UNorm; //todo
}

Format WGSwapChain::GetDepthStencilFormat() const
{
    return Format::D24UNormS8UInt; //todo
}

const RenderPass* WGSwapChain::GetRenderPass() const
{
    return nullptr; //todo
}

bool WGSwapChain::SetVsyncInterval(std::uint32_t vsyncInterval)
{
    return false; //todo
}


/*
 * ======= Private: =======
 */

bool WGSwapChain::ResizeBuffersPrimary(const Extent2D& resolution)
{
    return false; //TODO
}


} // /namespace LLGL



// ================================================================================
