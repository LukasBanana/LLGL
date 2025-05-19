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


namespace LLGL
{


LinuxGLSwapChainContextWayland::LinuxGLSwapChainContextWayland(LinuxGLContextWayland& context, Surface& surface) :
    GLSwapChainContext { context                 },
    context_           { context.GetEGLContext() }
{
    /* Get native window handle */
    NativeHandle nativeHandle = {};
    if (surface.GetNativeHandle(&nativeHandle, sizeof(nativeHandle)))
    {
        EGLDisplay display = eglGetCurrentDisplay();

        const EGLAttrib surfaceAttribs[] =
        {
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

        display_ = display;
        surface_ = surface;
    }
    else
        LLGL_TRAP("failed to get Wayland display and window from swap-chain surface");
}

bool LinuxGLSwapChainContextWayland::HasDrawable() const
{
    return (surface_ != 0);
}

bool LinuxGLSwapChainContextWayland::SwapBuffers()
{
    eglSwapBuffers(display_, surface_);
    return true;
}

void LinuxGLSwapChainContextWayland::Resize(const Extent2D& resolution)
{
    // dummy
}

bool LinuxGLSwapChainContextWayland::MakeCurrentEGLContext(LinuxGLSwapChainContextWayland *context)
{
    if (context)
        return eglMakeCurrent(context->display_, context->surface_, context->surface_, context->context_);
    else
        return eglMakeCurrent(eglGetDisplay(EGL_DEFAULT_DISPLAY), EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
}


} // /namespace LLGL

#endif // /LLGL_LINUX_ENABLE_WAYLAND



// ================================================================================