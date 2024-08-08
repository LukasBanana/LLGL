/*
 * Win32GLSwapChainContext.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "Win32GLSwapChainContext.h"
#include "Win32GLContext.h"
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
    return MakeUnique<Win32GLSwapChainContext>(static_cast<Win32GLContext&>(context), surface);
}

bool GLSwapChainContext::MakeCurrentUnchecked(GLSwapChainContext* context)
{
    return Win32GLSwapChainContext::MakeCurrentWGLContext(static_cast<Win32GLSwapChainContext*>(context));
}


/*
 * Win32GLSwapChainContext class
 */

Win32GLSwapChainContext::Win32GLSwapChainContext(Win32GLContext& context, Surface& surface) :
    GLSwapChainContext { context                 },
    hGLRC_             { context.GetGLRCHandle() }
{
    /* Get native window handle */
    NativeHandle nativeHandle = {};
    if (surface.GetNativeHandle(&nativeHandle, sizeof(nativeHandle)))
        hDC_ = ::GetDC(nativeHandle.window);
    else
        LLGL_TRAP("failed to get Win32 device context (HDC) from swap-chain surface");

    /* Select pixel format for device context if the GL context was originally created with a different surface */
    if (context.GetDCHandle() != hDC_)
        context.SelectPixelFormat(hDC_);
}

bool Win32GLSwapChainContext::HasDrawable() const
{
    return (hDC_ != nullptr);
}

bool Win32GLSwapChainContext::SwapBuffers()
{
    return (::SwapBuffers(hDC_) != FALSE);
}

void Win32GLSwapChainContext::Resize(const Extent2D& resolution)
{
    // do nothing (WGL context does not need to be resized)
}

bool Win32GLSwapChainContext::MakeCurrentWGLContext(Win32GLSwapChainContext* context)
{
    if (context)
        return (wglMakeCurrent(context->hDC_, context->hGLRC_) != GL_FALSE);
    else
        return (wglMakeCurrent(nullptr, nullptr) != GL_FALSE);
}


} // /namespace LLGL



// ================================================================================
