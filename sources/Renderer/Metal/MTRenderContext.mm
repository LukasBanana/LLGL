/*
 * MTRenderContext.mm
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "MTRenderContext.h"
#include "MTTypes.h"
#include "../TextureUtils.h"
#include <LLGL/Platform/NativeHandle.h>

#import <QuartzCore/CAMetalLayer.h>


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

    // We have no need of the internal drawing timer
    view_.paused                    = YES;
    view_.autoResizeDrawable        = NO;

    view_.colorPixelFormat          = renderPass_.GetColorAttachments()[0].pixelFormat;
    view_.depthStencilPixelFormat   = renderPass_.GetDepthStencilFormat();
    view_.sampleCount               = renderPass_.GetSampleCount();

    if (desc.vsync.enabled)
        view_.preferredFramesPerSecond = static_cast<NSInteger>(desc.vsync.refreshRate);

    OnSetDrawableResolution(desc.videoMode.resolution);
}

void MTRenderContext::Present()
{
    [view_ draw];
}

std::uint32_t MTRenderContext::GetSamples() const
{
    return static_cast<std::uint32_t>(renderPass_.GetSampleCount());
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

bool MTRenderContext::OnSetVideoMode(const VideoModeDescriptor &videoModeDesc)
{
    return true; // do nothing
}

bool MTRenderContext::OnSetDrawableResolution(const Extent2D& resolution)
{
    // We need to establish the scaling from the layer's bounds (which are
    // controlled by the NSView) to the drawable size in pixels. There is
    // only one scale factor. Verify that it works on both axes.
    CGFloat scale = resolution.width / view_.bounds.size.width;
    uint32_t scaledHeight = view_.bounds.size.height * scale + 0.5;
    if (scaledHeight != resolution.height)
        return false;

    if (scale != view_.layer.contentsScale)
        view_.layer.contentsScale = scale;

    CGSize drawableSize;
    drawableSize.width = resolution.width;
    drawableSize.height = resolution.height;
    if (!CGSizeEqualToSize(drawableSize, view_.drawableSize))
        view_.drawableSize = drawableSize;

    // Force the view to advance to the next drawable
    [view_ draw];

    return true;
}

bool MTRenderContext::OnSetVsync(const VsyncDescriptor& vsyncDesc)
{
    return false; //todo
}


} // /namespace LLGL



// ================================================================================
