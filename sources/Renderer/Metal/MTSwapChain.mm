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


#ifdef LLGL_OS_IOS

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

#endif // /LLGL_OS_IOS


namespace LLGL
{


MTSwapChain::MTSwapChain(
    id<MTLDevice>                   device,
    const SwapChainDescriptor&      desc,
    const std::shared_ptr<Surface>& surface,
    const RendererInfo&             rendererInfo)
:
    SwapChain   { desc         },
    renderPass_ { device, desc }
{
    /* Initialize surface for MetalKit view */
    SetOrCreateSurface(surface, SwapChain::BuildDefaultSurfaceTitle(rendererInfo), desc.resolution, desc.fullscreen);

    /* Allocate and initialize MetalKit view */
    view_ = AllocMTKViewAndInitWithSurface(device, GetSurface());

    /* Initialize color and depth buffer */
    view_.framebufferOnly           = NO; //TODO: make this optional with create/bind flag
    view_.colorPixelFormat          = renderPass_.GetColorAttachments()[0].pixelFormat;
    view_.depthStencilPixelFormat   = renderPass_.GetDepthStencilFormat();
    view_.sampleCount               = renderPass_.GetSampleCount();

    /* Show default surface */
    if (!surface)
        ShowSurface();
}

bool MTSwapChain::IsPresentable() const
{
    return true; //TODO
}

void MTSwapChain::Present()
{
    /* Present backbuffer */
    [view_ draw];

    /* Release mutable render pass as the view's render pass changes between backbuffers */
    if (nativeMutableRenderPass_ != nil)
    {
        [nativeMutableRenderPass_ release];
        nativeMutableRenderPass_ = nil;
    }
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

static NSInteger GetPrimaryDisplayRefreshRate()
{
    constexpr NSInteger defaultRefreshRate = 60;
    if (const Display* display = Display::GetPrimary())
        return static_cast<NSInteger>(display->GetDisplayMode().refreshRate);
    else
        return defaultRefreshRate;
}

bool MTSwapChain::SetVsyncInterval(std::uint32_t vsyncInterval)
{
    if (vsyncInterval > 0)
    {
        #ifdef LLGL_OS_MACOS
        /* Enable display sync in CAMetalLayer */
        [(CAMetalLayer*)[view_ layer] setDisplaySyncEnabled:YES];
        #endif

        /* Apply v-sync interval to display refresh rate */
        view_.preferredFramesPerSecond = GetPrimaryDisplayRefreshRate() / static_cast<NSInteger>(vsyncInterval);
    }
    else
    {
        #ifdef LLGL_OS_MACOS
        /* Disable display sync in CAMetalLayer */
        [(CAMetalLayer*)[view_ layer] setDisplaySyncEnabled:NO];
        #else
        /* Set preferred frame rate to default value */
        view_.preferredFramesPerSecond = GetPrimaryDisplayRefreshRate();
        #endif
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

#ifndef LLGL_OS_IOS

static NSView* GetContentViewFromNativeHandle(const NativeHandle& nativeHandle)
{
    if ([nativeHandle.responder isKindOfClass:[NSWindow class]])
    {
        /* Interpret responder as NSWindow */
        return [(NSWindow*)nativeHandle.responder contentView];
    }
    if ([nativeHandle.responder isKindOfClass:[NSView class]])
    {
        /* Interpret responder as NSView */
        return (NSView*)nativeHandle.responder;
    }
    LLGL_TRAP("NativeHandle::responder is neither of type NSWindow nor NSView for MTKView");
}

#endif

MTKView* MTSwapChain::AllocMTKViewAndInitWithSurface(id<MTLDevice> device, Surface& surface)
{
    MTKView* mtkView = nullptr;

    NativeHandle nativeHandle = {};
    GetSurface().GetNativeHandle(&nativeHandle, sizeof(nativeHandle));

    /* Create MetalKit view */
    #ifdef LLGL_OS_IOS

    LLGL_ASSERT_PTR(nativeHandle.view);
    UIView* contentView = nativeHandle.view;

    /* Allocate MetalKit view */
    mtkView = [[MTKView alloc] initWithFrame:contentView.frame device:device];

    /* Allocate view delegate to handle re-draw events */
    viewDelegate_ = [[MTSwapChainViewDelegate alloc] initWithCanvas:CastTo<Canvas>(GetSurface())];
    [viewDelegate_ mtkView:mtkView drawableSizeWillChange:mtkView.bounds.size];
    [mtkView setDelegate:viewDelegate_];

    #else // LLGL_OS_IOS

    NSView* contentView = GetContentViewFromNativeHandle(nativeHandle);

    /* Allocate MetalKit view */
    CGRect contentViewRect = [contentView frame];
    CGRect relativeViewRect = CGRectMake(0.0f, 0.0f, contentViewRect.size.width, contentViewRect.size.height);
    mtkView = [[MTKView alloc] initWithFrame:relativeViewRect device:device];

    #endif // /LLGL_OS_IOS

    /* Add MTKView as subview and register rotate/resize layout constraints */
    mtkView.translatesAutoresizingMaskIntoConstraints = NO;
    NSDictionary* viewsDictionary = @{@"mtkView":mtkView};
    [contentView addSubview:mtkView];
    [contentView addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"|[mtkView]|" options:0 metrics:nil views:viewsDictionary]];
    [contentView addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"V:|[mtkView]|" options:0 metrics:nil views:viewsDictionary]];

    return mtkView;
}

bool MTSwapChain::ResizeBuffersPrimary(const Extent2D& /*resolution*/)
{
    /* Invoke a redraw to force an update on the resized multisampled framebuffer */
    [view_ draw];
    return true;
}


} // /namespace LLGL



// ================================================================================
