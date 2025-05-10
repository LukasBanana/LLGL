/*
 * LinuxGLSwapChainContext.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "../GLSwapChainContext.h"
#include "LinuxGLSwapChainContextX11.h"

#if LLGL_LINUX_ENABLE_WAYLAND
#include "LinuxGLSwapChainContextWayland.h"
#endif

#include "../../../../Core/CoreUtils.h"
#include <LLGL/Platform/NativeHandle.h>
#include <wayland-egl-core.h>


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

    if (nativeHandle.type == NativeType::Wayland)
    {
        /* Create EGL swap-chain context for Wayland */
        return MakeUnique<LinuxGLSwapChainContextWayland>(static_cast<LinuxGLContextWayland&>(context), surface);
    }
    else
    #endif
    {
        /* Create GLX swap-chain context for X11 */
        return MakeUnique<LinuxGLSwapChainContextX11>(static_cast<LinuxGLContextX11&>(context), surface);
    }
}

bool GLSwapChainContext::MakeCurrentUnchecked(GLSwapChainContext* context)
{
    #if LLGL_LINUX_ENABLE_WAYLAND
    if (context != nullptr)
    {
        LinuxGLContext& contextLinuxGL = LLGL_CAST(LinuxGLContext&, context->GetGLContext());
        if (contextLinuxGL.GetNativeType() == OpenGL::RenderSystemNativeType::EGL)
            return LinuxGLSwapChainContextWayland::MakeCurrentEGLContext(static_cast<LinuxGLSwapChainContextWayland*>(context));
        else
            return LinuxGLSwapChainContextX11::MakeCurrentGLXContext(static_cast<LinuxGLSwapChainContextX11*>(context));
    }
    else
    {
        /* If there is no active GLX context, unset EGL context instead */
        if (glXGetCurrentContext() == nullptr)
            return LinuxGLSwapChainContextWayland::MakeCurrentEGLContext(nullptr);
        else
            return LinuxGLSwapChainContextX11::MakeCurrentGLXContext(nullptr);
    }
    #else
    return LinuxGLSwapChainContextX11::MakeCurrentGLXContext(static_cast<LinuxGLSwapChainContextX11*>(context));
    #endif
}

#endif

} // /namespace LLGL



// ================================================================================