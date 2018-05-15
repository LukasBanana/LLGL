/*
 * DbgRenderContext.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "DbgRenderContext.h"


namespace LLGL
{


DbgRenderContext::DbgRenderContext(RenderContext& instance) :
    instance { instance }
{
    ShareSurfaceAndVideoMode(instance);
}

void DbgRenderContext::Present()
{
    instance.Present();
}


/*
 * ======= Private: =======
 */

bool DbgRenderContext::OnSetVideoMode(const VideoModeDescriptor& videoModeDesc)
{
    return instance.SetVideoMode(videoModeDesc);
}

bool DbgRenderContext::OnSetVsync(const VsyncDescriptor& vsyncDesc)
{
    return instance.SetVsync(vsyncDesc);
}


} // /namespace LLGL



// ================================================================================
