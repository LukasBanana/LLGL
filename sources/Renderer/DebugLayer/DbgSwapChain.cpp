/*
 * DbgSwapChain.cpp
 * 
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "DbgSwapChain.h"
#include "DbgCore.h"


namespace LLGL
{


DbgSwapChain::DbgSwapChain(SwapChain& instance) :
    instance { instance }
{
    ShareSurfaceAndConfig(instance);
}

void DbgSwapChain::SetName(const char* name)
{
    DbgSetObjectName(*this, name);
}

void DbgSwapChain::Present()
{
    instance.Present();
}

std::uint32_t DbgSwapChain::GetSamples() const
{
    return instance.GetSamples();
}

Format DbgSwapChain::GetColorFormat() const
{
    return instance.GetColorFormat();
}

Format DbgSwapChain::GetDepthStencilFormat() const
{
    return instance.GetDepthStencilFormat();
}

bool DbgSwapChain::SetVsyncInterval(std::uint32_t vsyncInterval)
{
    return instance.SetVsyncInterval(vsyncInterval);
}

const RenderPass* DbgSwapChain::GetRenderPass() const
{
    return instance.GetRenderPass();
}

bool DbgSwapChain::ResizeBuffersPrimary(const Extent2D& resolution)
{
    return instance.ResizeBuffers(resolution);
}


} // /namespace LLGL



// ================================================================================
