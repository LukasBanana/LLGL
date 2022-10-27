/*
 * DbgRenderContext.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "DbgRenderContext.h"
#include "DbgCore.h"


namespace LLGL
{


DbgRenderContext::DbgRenderContext(RenderContext& instance) :
    instance { instance }
{
    ShareSurfaceAndConfig(instance);
}

void DbgRenderContext::SetName(const char* name)
{
    DbgSetObjectName(*this, name);
}

void DbgRenderContext::Present()
{
    instance.Present();
}

std::uint32_t DbgRenderContext::GetSamples() const
{
    return instance.GetSamples();
}

Format DbgRenderContext::GetColorFormat() const
{
    return instance.GetColorFormat();
}

Format DbgRenderContext::GetDepthStencilFormat() const
{
    return instance.GetDepthStencilFormat();
}

bool DbgRenderContext::SetVsyncInterval(std::uint32_t vsyncInterval)
{
    return instance.SetVsyncInterval(vsyncInterval);
}

const RenderPass* DbgRenderContext::GetRenderPass() const
{
    return instance.GetRenderPass();
}

bool DbgRenderContext::ResizeBuffersPrimary(const Extent2D& resolution)
{
    return instance.ResizeBuffers(resolution);
}


} // /namespace LLGL



// ================================================================================
