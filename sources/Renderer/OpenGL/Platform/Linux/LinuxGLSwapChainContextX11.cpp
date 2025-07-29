/*
 * LinuxGLSwapChainContextX11.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "LinuxGLSwapChainContextX11.h"
#include "LinuxGLContextX11.h"
#include "../../../../Core/CoreUtils.h"
#include "../../../../Core/Exception.h"
#include <LLGL/Platform/NativeHandle.h>


namespace LLGL
{


LinuxGLSwapChainContextX11::LinuxGLSwapChainContextX11(LinuxGLContextX11& context, Surface& surface) :
    GLSwapChainContext { context                 },
    glc_               { context.GetGLXContext() }
{
    /* Get native window handle */
    NativeHandle nativeHandle = {};
    if (surface.GetNativeHandle(&nativeHandle, sizeof(nativeHandle)))
    {
        dpy_ = nativeHandle.x11.display;
        wnd_ = nativeHandle.x11.window;
    }
    else
        LLGL_TRAP("failed to get X11 Display and Window from swap-chain surface");
}

bool LinuxGLSwapChainContextX11::HasDrawable() const
{
    return (wnd_ != 0);
}

bool LinuxGLSwapChainContextX11::SwapBuffers()
{
    GLSwapChainContext::MakeCurrent(this);
    glXSwapBuffers(dpy_, wnd_);
    return true;
}

void LinuxGLSwapChainContextX11::Resize(const Extent2D& resolution)
{
    // dummy
}

bool LinuxGLSwapChainContextX11::MakeCurrentGLXContext(LinuxGLSwapChainContextX11 *context)
{
    if (context)
        return glXMakeCurrent(context->dpy_, context->wnd_, context->glc_);
    else
        return glXMakeCurrent(nullptr, 0, 0);
}


} // /namespace LLGL



// ================================================================================