/*
 * LinuxGLSwapChainContextWayland.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#if LLGL_LINUX_ENABLE_WAYLAND

#include "LinuxGLContextWayland.h"
#include "LinuxGLSwapChainContextWayland.h"

#include <wayland-egl-core.h>

namespace LLGL {

LinuxWaylandGLSwapChainContext::LinuxWaylandGLSwapChainContext(LinuxGLContextWayland& context, Surface& surface) :
    GLSwapChainContext { context                 },
    glc_               { static_cast<EGLContext>(context.GetEGLContext()) }
{
    /* Get native window handle */
    NativeHandle nativeHandle = {};
    if (surface.GetNativeHandle(&nativeHandle, sizeof(nativeHandle)))
    {
        EGLDisplay display = eglGetCurrentDisplay();

        EGLAttrib surfaceAttribs[] = {
            EGL_RENDER_BUFFER, EGL_BACK_BUFFER,
            EGL_GL_COLORSPACE, EGL_GL_COLORSPACE_LINEAR,
            EGL_NONE
        };

        LLGL::Extent2D size = surface.GetContentSize();

        wl_egl_window* win = wl_egl_window_create(nativeHandle.wayland.window, size.width, size.height);
        if (!win)
            LLGL_TRAP("failed to create EGL window");

        EGLSurface surface = eglCreatePlatformWindowSurface(display, context.GetEGLConfig(), win, surfaceAttribs);
        if (surface == EGL_NO_SURFACE)
            LLGL_TRAP("failed to create EGL surface");

        dpy_ = display;
        wnd_ = surface;
    }
    else
        LLGL_TRAP("failed to get Wayland display and window from swap-chain surface");
}

bool LinuxWaylandGLSwapChainContext::HasDrawable() const
{
    return (wnd_ != 0);
}

bool LinuxWaylandGLSwapChainContext::SwapBuffers()
{
    eglSwapBuffers(dpy_, wnd_);
    return true;
}

void LinuxWaylandGLSwapChainContext::Resize(const Extent2D& resolution)
{
    // dummy
}

bool LinuxWaylandGLSwapChainContext::MakeCurrentEGLContext(LinuxWaylandGLSwapChainContext *context)
{
    if (context)
        return eglMakeCurrent(context->dpy_, context->wnd_, context->wnd_, context->glc_);
    else
        return eglMakeCurrent(nullptr, nullptr, nullptr, nullptr);
}

}

#endif