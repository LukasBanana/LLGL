/*
 * MTSwapChain.mm
 * 
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "MTSwapChain.h"
#include "MTTypes.h"
#include "../TextureUtils.h"
#include <LLGL/Platform/NativeHandle.h>

#import <QuartzCore/CAMetalLayer.h>


namespace LLGL
{


MTSwapChain::MTSwapChain(
    id<MTLDevice>                   device,
    const SwapChainDescriptor&      desc,
    const std::shared_ptr<Surface>& surface)
:
    SwapChain   { desc         },
    renderPass_ { desc, device }
{
    /* Create surface */
    SetOrCreateSurface(surface, desc.resolution, desc.fullscreen, nullptr);

    NativeHandle nativeHandle = {};
    GetSurface().GetNativeHandle(&nativeHandle, sizeof(nativeHandle));

    /* Create MetalKit view */
    #ifdef LLGL_OS_IOS

    //UIView* view = nativeHandle.view;
    CGRect screenBounds = [UIScreen mainScreen].nativeBounds;

    view_ = [[MTKView alloc] initWithFrame:screenBounds device:device];

    if (NSClassFromString(@"MTKView") != nullptr)
    {
        CALayer* layer = view_.layer;
        if (layer != nullptr && [layer isKindOfClass:NSClassFromString(@"CAMetalLayer")])
        {
            metalLayer_ = reinterpret_cast<CAMetalLayer*>(layer);
        }
    }

    #else

    NSWindow* wnd = nativeHandle.window;

    view_ = [[MTKView alloc] initWithFrame:wnd.frame device:device];

    [wnd setContentView:view_];
    [wnd.contentViewController setView:view_];

    #endif // /LLGL_OS_IOS

    /* Initialize color and depth buffer */
    //MTLPixelFormat colorFmt = metalLayer_.pixelFormat;

    view_.colorPixelFormat          = renderPass_.GetColorAttachments()[0].pixelFormat;
    view_.depthStencilPixelFormat   = renderPass_.GetDepthStencilFormat();
    view_.sampleCount               = renderPass_.GetSampleCount();
}

void MTSwapChain::Present()
{
    [view_ draw];
}

std::uint32_t MTSwapChain::GetSamples() const
{
    return static_cast<std::uint32_t>(renderPass_.GetSampleCount());
}

Format MTSwapChain::GetColorFormat() const
{
    return MTTypes::ToFormat(view_.colorPixelFormat);
}

Format MTSwapChain::GetDepthStencilFormat() const
{
    return MTTypes::ToFormat(view_.depthStencilPixelFormat);
}

const RenderPass* MTSwapChain::GetRenderPass() const
{
    return (&renderPass_);
}

bool MTSwapChain::SetVsyncInterval(std::uint32_t vsyncInterval)
{
    static const NSInteger defaultRefreshRate = 60;
    if (vsyncInterval > 0)
    {
        /* Apply v-sync interval to display refresh rate */
        if (auto display = Display::GetPrimary())
            view_.preferredFramesPerSecond = static_cast<NSInteger>(display->GetDisplayMode().refreshRate / vsyncInterval);
        else
            view_.preferredFramesPerSecond = defaultRefreshRate / static_cast<NSInteger>(vsyncInterval);
    }
    else
    {
        /* Set preferred frame rate to default value */
        view_.preferredFramesPerSecond = defaultRefreshRate;
    }
    return true;
}


/*
 * ======= Private: =======
 */

bool MTSwapChain::ResizeBuffersPrimary(const Extent2D& /*resolution*/)
{
    return true; // do nothing
}


} // /namespace LLGL



// ================================================================================
