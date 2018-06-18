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
    
    #if 1//TEST
    view_.colorPixelFormat          = MTLPixelFormatBGRA8Unorm_sRGB;
    view_.depthStencilPixelFormat   = MTLPixelFormatDepth32Float_Stencil8;
    #endif
}

MTRenderContext::~MTRenderContext()
{
}

void MTRenderContext::Present()
{
    [renderCmdEncoder_ endEncoding];
    [cmdBuffer_ presentDrawable:view_.currentDrawable];
    [cmdBuffer_ commit];
}

/* ----- Extended functions ----- */

void MTRenderContext::MakeCurrent(id<MTLCommandBuffer> cmdBuffer, id<MTLRenderCommandEncoder> renderCmdEncoder)
{
    cmdBuffer_          = cmdBuffer;
    renderCmdEncoder_   = renderCmdEncoder;
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
