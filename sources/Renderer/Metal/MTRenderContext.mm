/*
 * MTRenderContext.mm
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "MTRenderContext.h"
#include "MTTypes.h"
#include "../TextureUtils.h"
#include <LLGL/Platform/NativeHandle.h>


namespace LLGL
{


MTRenderContext::MTRenderContext(
    id<MTLDevice>                   device,
    RenderContextDescriptor         desc,
    const std::shared_ptr<Surface>& surface)
:
    renderPass_ { desc }
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
    view_.colorPixelFormat          = renderPass_.GetColorAttachments()[0].pixelFormat;
    view_.depthStencilPixelFormat   = renderPass_.GetDepthStencilFormat();
    view_.sampleCount               = renderPass_.GetSampleCount();

    if (desc.vsync.enabled)
        view_.preferredFramesPerSecond = static_cast<NSInteger>(desc.vsync.refreshRate);
}

void MTRenderContext::Present()
{
    [view_ draw];
}

std::uint32_t MTRenderContext::GetSamples() const
{
    return renderPass_.GetSampleCount();
}

Format MTRenderContext::GetColorFormat() const
{
    return MTTypes::ToFormat(view_.colorPixelFormat);
}

Format MTRenderContext::GetDepthStencilFormat() const
{
    return MTTypes::ToFormat(view_.depthStencilPixelFormat);
}

const RenderPass* MTRenderContext::GetRenderPass() const
{
    return (&renderPass_);
}


/*
 * ======= Private: =======
 */

bool MTRenderContext::OnSetVideoMode(const VideoModeDescriptor& videoModeDesc)
{
    return true; // do nothing
}

bool MTRenderContext::OnSetVsync(const VsyncDescriptor& vsyncDesc)
{
    return false; //todo
}


} // /namespace LLGL



// ================================================================================
