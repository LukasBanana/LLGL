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
    if (surface_ == nullptr)
    {
        window_ = window;
        surface_ = eglCreateWindowSurface(display_, config_, window_, nullptr);
        if (!surface_)
            LLGL_TRAP("eglCreateWindowSurface failed (%s)", EGLErrorToString());
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
