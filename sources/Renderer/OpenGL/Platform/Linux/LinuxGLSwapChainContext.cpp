/*
 * LinuxGLSwapChainContext.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "LinuxGLSwapChainContext.h"
#include "LinuxGLContext.h"
#include "../../../../Core/CoreUtils.h"
#include <LLGL/Platform/NativeHandle.h>


namespace LLGL
{


/*
 * GLSwapChainContext class
 */

std::unique_ptr<GLSwapChainContext> GLSwapChainContext::Create(GLContext& context, Surface& surface)
{
    return MakeUnique<LinuxGLSwapChainContext>(static_cast<LinuxGLContext&>(context), surface);
}

bool GLSwapChainContext::MakeCurrentUnchecked(GLSwapChainContext* context)
{
    return LinuxGLSwapChainContext::MakeCurrentGLXContext(static_cast<LinuxGLSwapChainContext*>(context));
}


/*
 * LinuxGLSwapChainContext class
 */

LinuxGLSwapChainContext::LinuxGLSwapChainContext(LinuxGLContext& context, Surface& surface) :
    GLSwapChainContext { context                 },
    glc_               { context.GetGLXContext() }
{
    /* Get native window handle */
    NativeHandle nativeHandle = {};
    if (surface.GetNativeHandle(&nativeHandle, sizeof(nativeHandle)))
    {
        dpy_ = nativeHandle.display;
        wnd_ = nativeHandle.window;
    }
    else
        throw std::runtime_error("failed to get X11 Display and Window from swap-chain surface");
}

bool LinuxGLSwapChainContext::SwapBuffers()
{
    glXSwapBuffers(dpy_, wnd_);
    return true;
}

bool LinuxGLSwapChainContext::MakeCurrentGLXContext(LinuxGLSwapChainContext* context)
{
    if (context)
        return glXMakeCurrent(context->dpy_, context->wnd_, context->glc_);
    else
        return glXMakeCurrent(nullptr, 0, 0);
}


} // /namespace LLGL



// ================================================================================
