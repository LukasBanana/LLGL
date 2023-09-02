/*
 * MacOSGLSwapChainContext.mm
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "MacOSGLSwapChainContext.h"
#include "MacOSGLContext.h"
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

bool GLSwapChainContext::MakeCurrentUnchecked(GLSwapChainContext* context)
{
    return MacOSGLSwapChainContext::MakeCurrentNSGLContext(static_cast<MacOSGLSwapChainContext*>(context));
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
    if (surface.GetNativeHandle(&nativeHandle, sizeof(nativeHandle)))
        view_ = [nativeHandle.window contentView];
    else
        throw std::runtime_error("failed to get NSWindow from swap-chain surface");
}

bool MacOSGLSwapChainContext::SwapBuffers()
{
    [ctx_ flushBuffer];
    return true;
}

bool MacOSGLSwapChainContext::MakeCurrentNSGLContext(MacOSGLSwapChainContext* context)
{
    if (context != nullptr)
    {
        /* Make context current */
        MacOSGLContext::MakeNSOpenGLContextCurrent(context->ctx_);

        /* 'setView' is deprecated since macOS 10.14 together with OpenGL in general, so suppress this deprecation warning */
        #pragma clang diagnostic push
        #pragma clang diagnostic ignored "-Wdeprecated-declarations"

        [context->ctx_ setView:context->view_];

        #pragma clang diagnostic pop

        [context->ctx_ update];
    }
    return true;
}


} // /namespace LLGL



// ================================================================================
