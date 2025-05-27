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
    display_           { context.GetEGLDisplay() },
    context_           { context.GetEGLContext() }
{
    /* Get native surface handle */
    NativeHandle nativeHandle = {};
    if (!surface.GetNativeHandle(&nativeHandle, sizeof(nativeHandle)))
        LLGL_TRAP("failed to get Wayland display and window from swap-chain surface");

    /* Get or create drawable surface */
    if (context.GetSharedEGLSurface() && context.GetSharedEGLSurface()->GetNativeWindow() != nullptr)
    {
        /* Share surface with main context */
        sharedSurface_ = context.GetSharedEGLSurface();
    }
    else
    {
        LLGL::Extent2D size = surface.GetContentSize();

        wl_egl_window* win = wl_egl_window_create(nativeHandle.wayland.window, size.width, size.height);
        if (!win)
            LLGL_TRAP("failed to create EGL window");

        /* Create custom surface if different native window is specified */
        sharedSurface_ = std::make_shared<LinuxSharedEGLSurface>(display_, context.GetEGLConfig(), win);
    }
}

bool LinuxGLSwapChainContextWayland::HasDrawable() const
{
    return (sharedSurface_ != 0);
}

bool LinuxGLSwapChainContextWayland::SwapBuffers()
{
    eglSwapBuffers(display_, sharedSurface_->GetEGLSurface());
    return true;
}

void LinuxGLSwapChainContextWayland::Resize(const Extent2D& resolution)
{
    // dummy
}

bool LinuxGLSwapChainContextWayland::MakeCurrentEGLContext(LinuxGLSwapChainContextWayland *context)
{
    if (context)
    {
        EGLSurface nativeSurface = context->sharedSurface_->GetEGLSurface();
        return eglMakeCurrent(context->display_, nativeSurface, nativeSurface, context->context_);
    }
    else
        return eglMakeCurrent(eglGetDisplay(EGL_DEFAULT_DISPLAY), EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
}


} // /namespace LLGL

#endif // /LLGL_LINUX_ENABLE_WAYLAND



// ================================================================================