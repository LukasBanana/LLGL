/*
 * MTRenderContext.mm
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "MTRenderContext.h"
#include <LLGL/Platform/NativeHandle.h>


namespace LLGL
{


/* ----- Common ----- */

MTRenderContext::MTRenderContext(
    id<MTLDevice>                   device,
    RenderContextDescriptor         desc,
    const std::shared_ptr<Surface>& surface)
{
    /* Create surface */
    SetOrCreateSurface(surface, desc.videoMode, nullptr);
    desc.videoMode = GetVideoMode();

    NativeHandle nativeHandle = {};
    GetSurface().GetNativeHandle(&nativeHandle);
    
    /* Create MetalKit view */
    NSWindow* wnd = nativeHandle.window;
    view_ = [[MTKView alloc] initWithFrame:wnd.frame device:device];
    [wnd setContentView:view_];
    [wnd.contentViewController setView:view_];
    
    //TODO: create swap-chain
}

MTRenderContext::~MTRenderContext()
{
}

void MTRenderContext::Present()
{
    [commandBuffer_ presentDrawable:view_.currentDrawable];
    [commandBuffer_ commit];
}

/* ----- Extended functions ----- */

void MTRenderContext::SetCommandBufferForPresent(id<MTLCommandBuffer> commandBuffer)
{
    commandBuffer_ = commandBuffer;
}


/*
 * ======= Private: =======
 */

bool MTRenderContext::OnSetVideoMode(const VideoModeDescriptor& videoModeDesc)
{
    return false; //todo
}

bool MTRenderContext::OnSetVsync(const VsyncDescriptor& vsyncDesc)
{
    return false; //todo
}


} // /namespace LLGL



// ================================================================================
