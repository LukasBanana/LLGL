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

#if LLGL_LINUX_ENABLE_WAYLAND
#include <wayland-egl-core.h>
#endif


namespace LLGL
{


/*
 * GLSwapChainContext class
 */

std::unique_ptr<GLSwapChainContext> GLSwapChainContext::Create(GLContext& context, Surface& surface)
{
#if LLGL_LINUX_ENABLE_WAYLAND
    NativeHandle nativeHandle = {};
    surface.GetNativeHandle(&nativeHandle, sizeof(nativeHandle));

    if (nativeHandle.type == NativeHandleType::Wayland)
    {
        return MakeUnique<LinuxWaylandGLSwapChainContext>(static_cast<LinuxGLContext&>(context), surface);
    }
    else
    {
        return MakeUnique<LinuxX11GLSwapChainContext>(static_cast<LinuxGLContext&>(context), surface);
    }
#else
    return MakeUnique<LinuxX11GLSwapChainContext>(static_cast<LinuxGLContext&>(context), surface);
#endif
}

bool GLSwapChainContext::MakeCurrentUnchecked(GLSwapChainContext* context)
{
#if LLGL_LINUX_ENABLE_WAYLAND
    LinuxGLContext& linuxContext = LLGL_CAST(LinuxGLContext&, context->GetGLContext());

    if (linuxContext.IsEGL())
        return LinuxWaylandGLSwapChainContext::MakeCurrentEGLContext(static_cast<LinuxWaylandGLSwapChainContext*>(context));
    else
        return LinuxX11GLSwapChainContext::MakeCurrentGLXContext(static_cast<LinuxX11GLSwapChainContext*>(context));
#else
    return LinuxX11GLSwapChainContext::MakeCurrentGLXContext(static_cast<LinuxX11GLSwapChainContext*>(context));
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

bool LinuxX11GLSwapChainContext::MakeCurrentGLXContext(LinuxX11GLSwapChainContext *context)
{
    if (context)
        return glXMakeCurrent(context->dpy_, context->wnd_, context->glc_);
    else
        return glXMakeCurrent(nullptr, 0, 0);
}

/*
 * LinuxWaylandGLSwapChainContext class
 */

#if LLGL_LINUX_ENABLE_WAYLAND

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

bool LinuxWaylandGLSwapChainContext::MakeCurrentEGLContext(LinuxWaylandGLSwapChainContext *context)
{
    if (context)
        return eglMakeCurrent(context->dpy_, context->wnd_, context->wnd_, context->glc_);
    else
        return eglMakeCurrent(nullptr, nullptr, nullptr, nullptr);
}

#endif

} // /namespace LLGL



// ================================================================================
