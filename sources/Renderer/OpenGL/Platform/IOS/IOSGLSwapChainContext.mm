/*
 * IOSGLSwapChainContext.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "IOSGLSwapChainContext.h"
#include "IOSGLContext.h"
#include <LLGL/TypeInfo.h>
#include "../../../../Core/Assertion.h"
#include "../../../../Core/CoreUtils.h"
#include <LLGL/Canvas.h>
#include <LLGL/Platform/NativeHandle.h>


@implementation IOSGLSwapChainViewController
{
    EAGLContext*    context_;
    LLGL::Canvas*   canvas_;
}

- (nonnull instancetype)initWithEAGLContext:(EAGLContext*)context withCanvas:(LLGL::Canvas&)canvas;
{
    self = [super init];
    if (self)
    {
        self->context_  = context;
        self->canvas_   = &canvas;
    }
    return self;
}

- (void)dealloc
{
    if ([EAGLContext currentContext] == self->context_)
        [EAGLContext setCurrentContext:nil];
    [super dealloc];
}

- (void)glkView:(GLKView*)view drawInRect:(CGRect)rect
{
    if (canvas_)
        canvas_->PostDraw();
}

@end


namespace LLGL
{


/*
 * GLSwapChainContext class
 */

std::unique_ptr<GLSwapChainContext> GLSwapChainContext::Create(GLContext& context, Surface& surface)
{
    return MakeUnique<IOSGLSwapChainContext>(static_cast<IOSGLContext&>(context), surface);
}

bool GLSwapChainContext::MakeCurrentUnchecked(GLSwapChainContext* context)
{
    return IOSGLSwapChainContext::MakeCurrentEGLContext(static_cast<IOSGLSwapChainContext*>(context));
}


/*
 * IOSGLSwapChainContext class
 */

static GLKViewDrawableColorFormat ToGLKViewColorFormat(int colorBits)
{
    if (colorBits == 16)
        return GLKViewDrawableColorFormatRGB565;
    else
        return GLKViewDrawableColorFormatRGBA8888; // default to 32 bits
}

static GLKViewDrawableDepthFormat ToGLKViewDepthFormat(int depthBits)
{
    if (depthBits == 0)
        return GLKViewDrawableDepthFormatNone;
    else if (depthBits == 16)
        return GLKViewDrawableDepthFormat16;
    else
        return GLKViewDrawableDepthFormat24; // default to 24 bits
}

static GLKViewDrawableStencilFormat ToGLKViewStencilFormat(int stencilBits)
{
    if (stencilBits == 0)
        return GLKViewDrawableStencilFormatNone;
    else
        return GLKViewDrawableStencilFormat8; // default to 8 bits
}

static GLKViewDrawableMultisample ToGLKViewMultisampleFormat(int samples)
{
    if (samples != 0)
        return GLKViewDrawableMultisample4X;
    else
        return GLKViewDrawableMultisampleNone; // default to no multi-sampling
}

IOSGLSwapChainContext::IOSGLSwapChainContext(IOSGLContext& context, Surface& surface) :
    GLSwapChainContext { context                  },
    context_           { context.GetEAGLContext() }
{
    Canvas& canvas = CastTo<Canvas>(surface);
    NativeHandle nativeHandle = {};
    surface.GetNativeHandle(&nativeHandle, sizeof(nativeHandle));

    LLGL_ASSERT_PTR(nativeHandle.view);
    UIView* canvasView = nativeHandle.view;

    /* Allocate GLKit view */
    view_ = [[GLKView alloc] initWithFrame:canvasView.frame context:context_];

    /* Allocate view controller (also inherts from GLKViewDelegate) to handle re-draw events */
    viewController_ = [[IOSGLSwapChainViewController alloc] initWithEAGLContext:context_ withCanvas:canvas];
    [viewController_ setView:view_];

    /* Configure GLKView with initial GL pixel format */
    const GLPixelFormat& pixelFormat = context.GetPixelFormat();
    view_.drawableColorFormat   = ToGLKViewColorFormat(pixelFormat.colorBits);
    view_.drawableDepthFormat   = ToGLKViewDepthFormat(pixelFormat.depthBits);
    view_.drawableStencilFormat = ToGLKViewStencilFormat(pixelFormat.stencilBits);
    view_.drawableMultisample   = ToGLKViewMultisampleFormat(pixelFormat.samples);

    //NSDictionary* viewsDictionary = @{@"glkView":view_};
    [canvasView addSubview:view_];
    [canvasView setAutoresizesSubviews:YES];
    //[canvasView addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"|[glkView]|" options:0 metrics:nil views:viewsDictionary]];
    //[canvasView addConstraints:[NSLayoutConstraint constraintsWithVisualFormat:@"V:|[glkView]|" options:0 metrics:nil views:viewsDictionary]];
}

bool IOSGLSwapChainContext::HasDrawable() const
{
    return (view_ != nullptr);
}

bool IOSGLSwapChainContext::SwapBuffers()
{
    return true; // dummy
}

void IOSGLSwapChainContext::Resize(const Extent2D& resolution)
{
    [view_ setNeedsDisplay];
    [view_ display];
}

bool IOSGLSwapChainContext::MakeCurrentEGLContext(IOSGLSwapChainContext* context)
{
    EAGLContext* contextEAGL = (context != nullptr ? context->context_ : nil);
    return ([EAGLContext setCurrentContext:contextEAGL] != NO);
}


} // /namespace LLGL



// ================================================================================
