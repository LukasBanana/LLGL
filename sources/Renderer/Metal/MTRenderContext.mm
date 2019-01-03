/*
 * MTRenderContext.mm
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "MTRenderContext.h"
#include "MTTypes.h"
#include <LLGL/Platform/NativeHandle.h>


namespace LLGL
{


static MTLPixelFormat GetColorMTLPixelFormat(int /*colorBits*/)
{
    return MTLPixelFormatBGRA8Unorm;
}

static MTLPixelFormat GetDepthStencilMTLPixelFormat(int depthBits, int stencilBits)
{
    #if 0 //TODO: graphics pipeline must get the correct type from the render context
    if (depthBits > 0 && stencilBits > 0)
    {
        if (depthBits == 32)
            return MTLPixelFormatDepth32Float_Stencil8;
    }
    else if (depthBits > 0)
    {
        if (depthBits == 32)
            return MTLPixelFormatDepth32Float;
        else if (depthBits == 16)
            return MTLPixelFormatDepth16Unorm;
    }
    else if (stencilBits > 0)
        return MTLPixelFormatStencil8;
    return MTLPixelFormatDepth24Unorm_Stencil8;
    #else
    return MTLPixelFormatDepth32Float_Stencil8;
    #endif
}

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
    
    /* Initialize color and depth buffer */
    view_.colorPixelFormat          = GetColorMTLPixelFormat(desc.videoMode.colorBits);
    view_.depthStencilPixelFormat   = GetDepthStencilMTLPixelFormat(desc.videoMode.depthBits, desc.videoMode.stencilBits);
    view_.sampleCount               = desc.multiSampling.SampleCount();
    
    if (desc.vsync.enabled)
        view_.preferredFramesPerSecond = static_cast<NSInteger>(desc.vsync.refreshRate);
}

void MTRenderContext::Present()
{
    [cmdBuffer_ presentDrawable:view_.currentDrawable];
    [cmdBuffer_ commit];
    [view_ draw]; //really required???
}

Format MTRenderContext::QueryColorFormat() const
{
    return MTTypes::ToFormat(view_.colorPixelFormat);
}

Format MTRenderContext::QueryDepthStencilFormat() const
{
    return MTTypes::ToFormat(view_.depthStencilPixelFormat);
}

const RenderPass* MTRenderContext::GetRenderPass() const
{
    return nullptr; //TODO
}

/* ----- Extended functions ----- */

void MTRenderContext::MakeCurrent(id<MTLCommandBuffer> cmdBuffer)
{
    cmdBuffer_ = cmdBuffer;
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
