/*
 * DbgRenderContext.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
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

/* ----- Configuration ----- */

void DbgRenderContext::SetVideoMode(const VideoModeDescriptor& videoModeDesc)
{
    instance.SetVideoMode(videoModeDesc);
    RenderContext::SetVideoMode(videoModeDesc);
}

void DbgRenderContext::SetVsync(const VsyncDescriptor& vsyncDesc)
{
    instance.SetVsync(vsyncDesc);
}


} // /namespace LLGL



// ================================================================================
