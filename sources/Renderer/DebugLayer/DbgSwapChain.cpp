/*
 * DbgSwapChain.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
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
