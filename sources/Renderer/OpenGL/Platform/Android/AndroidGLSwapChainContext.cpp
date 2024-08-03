/*
 * AndroidGLSwapChainContext.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "AndroidGLSwapChainContext.h"
#include "AndroidGLContext.h"
#include "AndroidGLCore.h"
#include "../../../../Core/CoreUtils.h"
#include "../../../../Core/Exception.h"
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
    NativeHandle nativeHandle = {};
    surface.GetNativeHandle(&nativeHandle, sizeof(nativeHandle));

    /* Get or create drawable surface */
    if (context.GetSharedEGLSurface() && nativeHandle.window == context.GetSharedEGLSurface()->GetNativeWindow())
    {
        /* Share surface with main context */
        surface_ = context.GetSharedEGLSurface();
    }
    else
    {
        //TODO: can this case ever happen on Android?
        /* Create custom surface if different native window is specified */
        surface_ = std::make_shared<AndroidSharedEGLSurface>(display_, context.GetEGLConfig(), nativeHandle.window);
    }
}

bool AndroidGLSwapChainContext::SwapBuffers()
{
    eglSwapBuffers(display_, surface_->GetEGLSurface());
    return true;
}

void AndroidGLSwapChainContext::Resize(const Extent2D& resolution)
{
    // dummy
}

bool AndroidGLSwapChainContext::MakeCurrentEGLContext(AndroidGLSwapChainContext* context)
{
    if (context)
    {
        EGLSurface nativeSurface = context->surface_->GetEGLSurface();
        return eglMakeCurrent(context->display_, nativeSurface, nativeSurface, context->context_);
    }
    else
        return eglMakeCurrent(eglGetDisplay(EGL_DEFAULT_DISPLAY), EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
}


} // /namespace LLGL



// ================================================================================
