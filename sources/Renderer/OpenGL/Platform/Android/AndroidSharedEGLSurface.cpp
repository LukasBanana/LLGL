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
    display_ { display                                                  },
    surface_ { eglCreateWindowSurface(display, config, window, nullptr) },
    window_  { window                                                   }
{
    if (!surface_)
        LLGL_TRAP("eglCreateWindowSurface failed (%s)", EGLErrorToString());
}

AndroidSharedEGLSurface::~AndroidSharedEGLSurface()
{
    eglDestroySurface(display_, surface_);
}


} // /namespace LLGL



// ================================================================================
