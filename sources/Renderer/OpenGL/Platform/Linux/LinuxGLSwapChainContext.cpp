/*
 * LinuxGLSwapChainContext.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "LinuxGLSwapChainContext.h"
#include "LinuxGLContext.h"
#include "../../../../Core/CoreUtils.h"
#include "../../../../Core/Exception.h"
#include <LLGL/Platform/NativeHandle.h>

#ifdef LLGL_LINUX_ENABLE_WAYLAND
#include <wayland-egl-core.h>
#endif


namespace LLGL
{


/*
 * GLSwapChainContext class
 */

std::unique_ptr<GLSwapChainContext> GLSwapChainContext::Create(GLContext& context, Surface& surface)
{
#ifdef LLGL_LINUX_ENABLE_WAYLAND
    if (surface.IsWayland()) {
        return MakeUnique<LinuxWaylandGLSwapChainContext>(static_cast<LinuxGLContext&>(context), surface);
    } else {
        return MakeUnique<LinuxX11GLSwapChainContext>(static_cast<LinuxGLContext&>(context), surface);
    }
#else
    return MakeUnique<LinuxX11GLSwapChainContext>(static_cast<LinuxGLContext&>(context), surface);
#endif
}


/*
 * LinuxX11GLSwapChainContext class
 */

LinuxX11GLSwapChainContext::LinuxX11GLSwapChainContext(LinuxGLContext& context, Surface& surface) :
    GLSwapChainContext { context                 },
    glc_               { static_cast<GLXContext>(context.GetGLXContext()) }
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

bool LinuxX11GLSwapChainContext::HasDrawable() const
{
    return (wnd_ != 0);
}

bool LinuxX11GLSwapChainContext::SwapBuffers()
{
    glXSwapBuffers(dpy_, wnd_);
    return true;
}

void LinuxX11GLSwapChainContext::Resize(const Extent2D& resolution)
{
    // dummy
}

bool LinuxX11GLSwapChainContext::MakeCurrentUnchecked()
{
    return glXMakeCurrent(dpy_, wnd_, glc_);       
}

bool LinuxX11GLSwapChainContext::Destroy() {
    return glXMakeCurrent(nullptr, 0, 0);
}

/*
 * LinuxWaylandGLSwapChainContext class
 */

#ifdef LLGL_LINUX_ENABLE_WAYLAND

LinuxWaylandGLSwapChainContext::LinuxWaylandGLSwapChainContext(LinuxGLContext& context, Surface& surface) :
    GLSwapChainContext { context                 },
    glc_               { static_cast<EGLContext>(context.GetGLXContext()) }
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

        EGLSurface surface = eglCreatePlatformWindowSurface(display, context.GetEGLConfig(), win, surfaceAttribs);

        dpy_ = display;
        wnd_ = surface;
    }
    else
        LLGL_TRAP("failed to get X11 Display and Window from swap-chain surface");
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

bool LinuxWaylandGLSwapChainContext::MakeCurrentUnchecked()
{
    return eglMakeCurrent(dpy_, wnd_, wnd_, glc_);
}

bool LinuxWaylandGLSwapChainContext::Destroy() {
    return eglMakeCurrent(nullptr, nullptr, nullptr, nullptr);
}

#endif

} // /namespace LLGL



// ================================================================================
