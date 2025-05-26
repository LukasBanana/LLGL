/*
 * LinuxSharedEGLSurface.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#if LLGL_LINUX_ENABLE_WAYLAND

#include "LinuxSharedEGLSurface.h"
#include "LinuxGLCore.h"
#include "../../../../Core/Exception.h"


namespace LLGL
{


LinuxSharedEGLSurface::LinuxSharedEGLSurface(EGLDisplay display, EGLConfig config, wl_egl_window* window) :
    display_ { display },
    config_  { config  }
{
    InitEGLSurface(window);
}

LinuxSharedEGLSurface::~LinuxSharedEGLSurface()
{
    DestroyEGLSurface();
}

void LinuxSharedEGLSurface::InitEGLSurface(wl_egl_window* window)
{
    /* Destroy previous surface if the window has changed */
    if (window_ != window)
        DestroyEGLSurface();

    if (surface_ == nullptr)
    {
        /* Store new window (null or non-null accepted) */
        window_ = window;
        if (window != nullptr)
        {
            /* Create an EGLSurface with a native window */
            surface_ = eglCreatePlatformWindowSurface(display_, config_, window_, nullptr);
            if (!surface_)
                LLGL_TRAP("eglCreateWindowSurface failed (%s)", EGLErrorToString());
        }
        else
        {
            /* Create an EGLSurface with a Pbuffer */
            const EGLint attribs[] =
            {
                EGL_LARGEST_PBUFFER,    EGL_TRUE,
                EGL_NONE
            };
            surface_ = eglCreatePbufferSurface(display_, config_, attribs);
            if (!surface_)
                LLGL_TRAP("eglCreatePbufferSurface failed (%s)", EGLErrorToString());
        }
    }
}

void LinuxSharedEGLSurface::DestroyEGLSurface()
{
    if (surface_ != nullptr)
    {
        eglDestroySurface(display_, surface_);
        surface_ = nullptr;
    }
}


} // /namespace LLGL

#endif

// ================================================================================
