/*
 * MTSwapChain.mm
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "MTSwapChain.h"
#include "MTTypes.h"
#include "RenderState/MTRenderPass.h"
#include "../TextureUtils.h"
#include "../../Core/Assertion.h"
#include <LLGL/Platform/NativeHandle.h>
#include <LLGL/TypeInfo.h>

#import <QuartzCore/CAMetalLayer.h>


@implementation MTSwapChainViewDelegate
{
    LLGL::Canvas* canvas_;
}

-(nonnull instancetype)initWithCanvas:(LLGL::Canvas&)canvas;
{
    self = [super init];
    if (self)
        canvas_ = &canvas;
    return self;
}

- (void)drawInMTKView:(nonnull MTKView *)view
{
    if (canvas_)
        canvas_->PostDraw();
}

- (void)mtkView:(nonnull MTKView *)view drawableSizeWillChange:(CGSize)size
{
    // dummy
}

@end


namespace LLGL
{


MTSwapChain::MTSwapChain(
    id<MTLDevice>                   device,
    const SwapChainDescriptor&      desc,
    const std::shared_ptr<Surface>& surface)
:
    SwapChain   { desc         },
    renderPass_ { device, desc }
{
    /* Initialize surface for MetalKit view */
    SetOrCreateSurface(surface, desc.resolution, desc.fullscreen, nullptr);

    /* Allocate and initialize MetalKit view */
    view_ = AllocMTKViewAndInitWithSurface(device, GetSurface());

    /* Initialize color and depth buffer */
    view_.framebufferOnly           = NO; //TODO: make this optional with create/bind flag
    view_.colorPixelFormat          = renderPass_.GetColorAttachments()[0].pixelFormat;
    view_.depthStencilPixelFormat   = renderPass_.GetDepthStencilFormat();
    view_.sampleCount               = renderPass_.GetSampleCount();
}

void MTSwapChain::Present()
{
    [view_ draw];
}

std::uint32_t MTSwapChain::GetCurrentSwapIndex() const
{
    return 0; // dummy
}

std::uint32_t MTSwapChain::GetNumSwapBuffers() const
{
    return 1; // dummy
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

MTLRenderPassDescriptor* MTSwapChain::GetAndUpdateNativeRenderPass(
    const MTRenderPass& renderPass,
    std::uint32_t       numClearValues,
    const ClearValue*   clearValues)
{
    /* Create copy of native render pass descriptor for the first time */
    if (nativeMutableRenderPass_ == nil)
        nativeMutableRenderPass_ = [GetNativeRenderPass() copy];

    /* Update mutable render pass with clear values */
    if (renderPass.GetColorAttachments().size() == 1)
        renderPass.UpdateNativeRenderPass(nativeMutableRenderPass_, numClearValues, clearValues);

    return nativeMutableRenderPass_;
}


/*
 * ======= Private: =======
 */

MTKView* MTSwapChain::AllocMTKViewAndInitWithSurface(id<MTLDevice> device, Surface& surface)
{
    MTKView* mtkView = nullptr;

    NativeHandle nativeHandle = {};
    GetSurface().GetNativeHandle(&nativeHandle, sizeof(nativeHandle));

    /* Create MetalKit view */
    #ifdef LLGL_OS_IOS

    LLGL_ASSERT_PTR(nativeHandle.view);
    UIView* canvasView = nativeHandle.view;

    /* Allocate MetalKit view */
    mtkView = [[MTKView alloc] initWithFrame:canvasView.frame device:device];

    /* Allocate view delegate to handle re-draw events */
    viewDelegate_ = [[MTSwapChainViewDelegate alloc] initWithCanvas:CastTo<Canvas>(GetSurface())];
    [viewDelegate_ mtkView:mtkView drawableSizeWillChange:mtkView.bounds.size];
    [mtkView setDelegate:viewDelegate_];

    /* Register rotate/resize layout constraints */
    mtkView.translatesAutoresizingMaskIntoConstraints = NO;
    NSDictionary* viewsDictionary = @{@"mtkView":mtkView};
    [canvasView addSubview:mtkView];
    [canvasView addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"|[mtkView]|" options:0 metrics:nil views:viewsDictionary]];
    [canvasView addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"V:|[mtkView]|" options:0 metrics:nil views:viewsDictionary]];

    #else // LLGL_OS_IOS

    if ([nativeHandle.responder isKindOfClass:[NSWindow class]])
    {
        NSWindow* contentWindow = (NSWindow*)nativeHandle.responder;

        /* Allocate MetalKit view */
        CGRect contentViewRect = [[contentWindow contentView] frame];
        mtkView = [[MTKView alloc] initWithFrame:contentViewRect device:device];

        /* Replace content view of input window with MTKView */
        [contentWindow setContentView:mtkView];
        [contentWindow.contentViewController setView:mtkView];
    }
    else if ([nativeHandle.responder isKindOfClass:[NSView class]])
    {
        NSView* contentView = (NSView*)nativeHandle.responder;

        /* Allocate MetalKit view */
        CGRect contentViewRect = [contentView frame];
        mtkView = [[MTKView alloc] initWithFrame:contentViewRect device:device];

        /* Add MTKView as subview to the input view */
        [contentView addSubview:mtkView];
    }
    else
        LLGL_TRAP("NativeHandle::responder is neither of type NSWindow nor NSView for MTKView");

    #endif // /LLGL_OS_IOS

    return mtkView;
}

bool MTSwapChain::ResizeBuffersPrimary(const Extent2D& /*resolution*/)
{
    return true; // do nothing
}


} // /namespace LLGL



// ================================================================================
