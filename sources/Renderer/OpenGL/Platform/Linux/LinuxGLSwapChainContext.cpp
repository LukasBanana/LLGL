/*
 * LinuxGLSwapChainContext.cpp
 *
 * Copyright (c) 2025 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "../GLSwapChainContext.h"
#include "LinuxGLSwapChainContextX11.h"

#if LLGL_LINUX_ENABLE_WAYLAND
#include "LinuxGLSwapChainContextWayland.h"
#endif

#include "../../../../Core/CoreUtils.h"
#include <LLGL/Platform/NativeHandle.h>


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

    if (nativeHandle.type == NativeHandleType::Wayland)
    {
        return MakeUnique<LinuxWaylandGLSwapChainContext>(static_cast<LinuxGLContextWayland&>(context), surface);
    }
    else
    {
        return MakeUnique<LinuxX11GLSwapChainContext>(static_cast<LinuxGLContextX11&>(context), surface);
    }
#else
    return MakeUnique<LinuxX11GLSwapChainContext>(static_cast<LinuxGLContextX11&>(context), surface);
#endif
}

bool GLSwapChainContext::MakeCurrentUnchecked(GLSwapChainContext* context)
{
#if LLGL_LINUX_ENABLE_WAYLAND
    LinuxGLContext& linuxContext = LLGL_CAST(LinuxGLContext&, context->GetGLContext());

    if (linuxContext.IsWayland())
        return LinuxWaylandGLSwapChainContext::MakeCurrentEGLContext(static_cast<LinuxWaylandGLSwapChainContext*>(context));
    else
        return LinuxX11GLSwapChainContext::MakeCurrentGLXContext(static_cast<LinuxX11GLSwapChainContext*>(context));
#else
    return LinuxX11GLSwapChainContext::MakeCurrentGLXContext(static_cast<LinuxX11GLSwapChainContext*>(context));
#endif
}

}

// ================================================================================
