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
#include <LLGL/TypeInfo.h>
#include <LLGL/Canvas.h>


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
 * CanvasEventListener
 */

class AndroidGLSwapChainContext::CanvasEventListener final : public Canvas::EventListener
{

    public:

        CanvasEventListener(AndroidGLSwapChainContext* context);

        void OnInit(Canvas& sender) override;
        void OnDestroy(Canvas& sender) override;

    private:

        AndroidGLSwapChainContext* context_ = nullptr;

};

AndroidGLSwapChainContext::CanvasEventListener::CanvasEventListener(AndroidGLSwapChainContext* context) :
    context_ { context }
{
}

void AndroidGLSwapChainContext::CanvasEventListener::OnInit(Canvas& sender)
{
    /* Re-initialize the shared EGLSurface when the ANativeWindow is re-initialized */
    context_->InitEGLSurface(sender);
    GLSwapChainContext::MakeCurrent(context_);
}

void AndroidGLSwapChainContext::CanvasEventListener::OnDestroy(Canvas& /*sender*/)
{
    /* Destroy the shared EGLSurface when the ANativeWindow is destroyed */
    context_->DestroyEGLSurface();
    GLSwapChainContext::MakeCurrent(nullptr);
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
        sharedSurface_ = context.GetSharedEGLSurface();
    }
    else
    {
        /* Create custom surface if different native window is specified */
        sharedSurface_ = std::make_shared<AndroidSharedEGLSurface>(display_, context.GetEGLConfig(), nativeHandle.window);
    }

    /* Register event listener to re-create EGLSurface when the app destroys and re-initializes the ANativeWindow during pausing/resuming the app */
    CastTo<Canvas>(surface).AddEventListener(std::make_shared<CanvasEventListener>(this));
}

bool AndroidGLSwapChainContext::HasDrawable() const
{
    return (sharedSurface_->GetEGLSurface() != nullptr);
}

bool AndroidGLSwapChainContext::SwapBuffers()
{
    eglSwapBuffers(display_, sharedSurface_->GetEGLSurface());
    return true;
}

void AndroidGLSwapChainContext::Resize(const Extent2D& resolution)
{
    // dummy
}

void AndroidGLSwapChainContext::InitEGLSurface(Surface& surface)
{
    NativeHandle nativeHandle = {};
    surface.GetNativeHandle(&nativeHandle, sizeof(nativeHandle));
    sharedSurface_->InitEGLSurface(nativeHandle.window);
}

void AndroidGLSwapChainContext::DestroyEGLSurface()
{
    sharedSurface_->DestroyEGLSurface();
}

bool AndroidGLSwapChainContext::MakeCurrentEGLContext(AndroidGLSwapChainContext* context)
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



// ================================================================================
