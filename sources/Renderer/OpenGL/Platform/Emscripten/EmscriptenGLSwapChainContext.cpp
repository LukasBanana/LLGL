/*
 * EmscriptenGLSwapChainContext.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "EmscriptenGLSwapChainContext.h"
#include "EmscriptenGLContext.h"
#include "../../../../Core/CoreUtils.h"
#include <LLGL/Platform/NativeHandle.h>


namespace LLGL
{


/*
 * GLSwapChainContext class
 */

std::unique_ptr<GLSwapChainContext> GLSwapChainContext::Create(GLContext& context, Surface& surface)
{
    return MakeUnique<EmscriptenGLSwapChainContext>(static_cast<EmscriptenGLContext&>(context), surface);
}

bool GLSwapChainContext::MakeCurrentUnchecked(GLSwapChainContext* context)
{
    return EmscriptenGLSwapChainContext::MakeCurrentEGLContext(static_cast<EmscriptenGLSwapChainContext*>(context));
}


/*
 * EmscriptenGLSwapChainContext class
 */

EmscriptenGLSwapChainContext::EmscriptenGLSwapChainContext(EmscriptenGLContext& context, Surface& surface) :
    GLSwapChainContext { context                 },
    display_           { context.GetEGLDisplay() },
    context_           { context.GetEGLContext() }
{
    /* Get native surface handle */
    NativeHandle nativeHandle;
    surface.GetNativeHandle(&nativeHandle, sizeof(nativeHandle));

    /* Create drawable surface */
    surface_ = eglCreateWindowSurface(display_, context.GetEGLConfig(), nativeHandle.window, nullptr);
    if (!surface_)
        throw std::runtime_error("eglCreateWindowSurface failed");
}

EmscriptenGLSwapChainContext::~EmscriptenGLSwapChainContext()
{
    eglDestroySurface(display_, surface_);
}

bool EmscriptenGLSwapChainContext::SwapBuffers()
{
    eglSwapBuffers(display_, surface_);
    return true;
}

void EmscriptenGLSwapChainContext::Resize(const Extent2D& resolution)
{
    // dummy
}

bool EmscriptenGLSwapChainContext::MakeCurrentEGLContext(EmscriptenGLSwapChainContext* context)
{
    if (context)
        return eglMakeCurrent(context->display_, context->surface_, context->surface_, context->context_);
    else
        return eglMakeCurrent(eglGetDisplay(EGL_DEFAULT_DISPLAY), EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
}


} // /namespace LLGL



// ================================================================================
