/*
 * AndroidGLSwapChainContext.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "AndroidGLSwapChainContext.h"
#include "AndroidGLContext.h"
#include "../../../../Core/Helper.h"
#include <LLGL/Platform/NativeHandle.h>


namespace LLGL
{


/*
 * GLSwapChainContext class
 */

std::unique_ptr<GLSwapChainContext> GLSwapChainContext::Create(GLContext& context, Surface& surface)
{
    return MakeUnique<AndroidGLSwapChainContext>(static_cast<AndroidGLContext&>(context), surface);
}

bool GLSwapChainContext::MakeCurrentUnchecked(GLSwapChainContext* context)
{
    return AndroidGLSwapChainContext::MakeCurrentEGLContext(static_cast<AndroidGLSwapChainContext*>(context));
}


/*
 * AndroidGLSwapChainContext class
 */

AndroidGLSwapChainContext::AndroidGLSwapChainContext(AndroidGLContext& context, Surface& surface) :
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

AndroidGLSwapChainContext::~AndroidGLSwapChainContext()
{
    eglDestroySurface(display_, surface_);
}

bool AndroidGLSwapChainContext::SwapBuffers()
{
    eglSwapBuffers(display_, surface_);
    return true;
}

bool AndroidGLSwapChainContext::MakeCurrentEGLContext(AndroidGLSwapChainContext* context)
{
    if (context)
        return eglMakeCurrent(context->display_, context->surface_, context->surface_, context->context_);
    else
        return eglMakeCurrent(eglGetDisplay(EGL_DEFAULT_DISPLAY), EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
}


} // /namespace LLGL



// ================================================================================
