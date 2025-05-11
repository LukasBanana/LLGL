/*
 * MacOSGLSwapChainContext.mm
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "MacOSGLSwapChainContext.h"
#include "MacOSGLContext.h"
#include "../../../../Core/Exception.h"
#include "../../../../Core/CoreUtils.h"
#include <LLGL/Platform/NativeHandle.h>


namespace LLGL
{


/*
 * GLSwapChainContext class
 */

std::unique_ptr<GLSwapChainContext> GLSwapChainContext::Create(GLContext& context, Surface& surface)
{
    return MakeUnique<MacOSGLSwapChainContext>(static_cast<MacOSGLContext&>(context), surface);
}


/*
 * MacOSGLSwapChainContext class
 */

MacOSGLSwapChainContext::MacOSGLSwapChainContext(MacOSGLContext& context, Surface& surface) :
    GLSwapChainContext { context                  },
    ctx_               { context.GetNSGLContext() }
{
    /* Get native window handle */
    NativeHandle nativeHandle = {};
    surface.GetNativeHandle(&nativeHandle, sizeof(nativeHandle));
    if ([nativeHandle.responder isKindOfClass:[NSWindow class]])
        view_ = [(NSWindow*)nativeHandle.responder contentView];
    else if ([nativeHandle.responder isKindOfClass:[NSView class]])
        view_ = (NSView*)nativeHandle.responder;
    else
        LLGL_TRAP("NativeHandle::responder is neither of type NSWindow nor NSView for GLKView");
}

bool MacOSGLSwapChainContext::HasDrawable() const
{
    return (view_ != nullptr);
}

bool MacOSGLSwapChainContext::SwapBuffers()
{
    [ctx_ flushBuffer];
    return true;
}

void MacOSGLSwapChainContext::Resize(const Extent2D& resolution)
{
    [ctx_ update];
}

bool MacOSGLSwapChainContext::MakeCurrentUnchecked()
{
    /* Make context current */
    MacOSGLContext::MakeNSOpenGLContextCurrent(ctx_);

    /* We have to flush the previous content or it will be lost */
    [ctx_ flushBuffer];

    /* 'setView' is deprecated since macOS 10.14 together with OpenGL in general, so suppress this deprecation warning */
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wdeprecated-declarations"

    [ctx_ setView:view_];

    #pragma clang diagnostic pop

    [ctx_ update];
}


} // /namespace LLGL



// ================================================================================
