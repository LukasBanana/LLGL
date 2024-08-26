/*
 * AndroidSharedEGLSurface.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "AndroidSharedEGLSurface.h"
#include "AndroidGLCore.h"
#include "../../../../Core/Exception.h"


namespace LLGL
{


AndroidSharedEGLSurface::AndroidSharedEGLSurface(EGLDisplay display, EGLConfig config, ANativeWindow* window) :
    display_ { display },
    config_  { config  }
{
    InitEGLSurface(window);
}

AndroidSharedEGLSurface::~AndroidSharedEGLSurface()
{
    DestroyEGLSurface();
}

void AndroidSharedEGLSurface::InitEGLSurface(ANativeWindow* window)
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
            surface_ = eglCreateWindowSurface(display_, config_, window_, nullptr);
            if (!surface_)
                LLGL_TRAP("eglCreateWindowSurface failed (%s)", EGLErrorToString());
        }
        else
        {
            /* Create an EGLSurface with a Pbuffer */
            const EGLint attribs[] =
            {
                EGL_LARGEST_PBUFFER,    EGL_TRUE,
                EGL_MIPMAP_TEXTURE,     EGL_TRUE,
                EGL_TEXTURE_TARGET,     EGL_TEXTURE_2D,
                EGL_NONE
            };
            surface_ = eglCreatePbufferSurface(display_, config_, attribs);
            if (!surface_)
                LLGL_TRAP("eglCreatePbufferSurface failed (%s)", EGLErrorToString());
        }
    }
}

void AndroidSharedEGLSurface::DestroyEGLSurface()
{
    if (surface_ != nullptr)
    {
        eglDestroySurface(display_, surface_);
        surface_ = nullptr;
    }
}


} // /namespace LLGL



// ================================================================================
