/*
 * DbgRenderContext.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "DbgRenderContext.h"


namespace LLGL
{


DbgRenderContext::DbgRenderContext(RenderContext& instance) :
    instance { instance }
{
    ShareSurfaceAndConfig(instance);
}

void DbgRenderContext::Present()
{
    instance.Present();
}

Format DbgRenderContext::QueryColorFormat() const
{
    return instance.QueryColorFormat();
}

Format DbgRenderContext::QueryDepthStencilFormat() const
{
    return instance.QueryDepthStencilFormat();
}

const RenderPass* DbgRenderContext::GetRenderPass() const
{
    return instance.GetRenderPass();
}


/*
 * ======= Private: =======
 */

bool DbgRenderContext::OnSetVideoMode(const VideoModeDescriptor& videoModeDesc)
{
    auto result = instance.SetVideoMode(videoModeDesc);
    ShareSurfaceAndConfig(instance);
    return result;
}

bool DbgRenderContext::OnSetVsync(const VsyncDescriptor& vsyncDesc)
{
    auto result = instance.SetVsync(vsyncDesc);
    ShareSurfaceAndConfig(instance);
    return result;
}


} // /namespace LLGL



// ================================================================================
