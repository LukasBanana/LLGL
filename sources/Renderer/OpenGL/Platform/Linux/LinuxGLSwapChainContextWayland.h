/*
 * LinuxGLSwapChainContextWayland.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_LINUX_GL_SWAP_CHAIN_CONTEXT_WAYLAND_H
#define LLGL_LINUX_GL_SWAP_CHAIN_CONTEXT_WAYLAND_H

#if LLGL_LINUX_ENABLE_WAYLAND

#include "LinuxGLContextWayland.h"
#include "../GLSwapChainContext.h"
#include "../../OpenGL.h"

#include <wayland-client.h>

namespace LLGL {

class LinuxWaylandGLSwapChainContext final : public GLSwapChainContext
{

    public:

        LinuxWaylandGLSwapChainContext(LinuxGLContextWayland& context, Surface& surface);

        bool HasDrawable() const override;
        bool SwapBuffers() override;
        void Resize(const Extent2D& resolution) override;
    public:

        static bool MakeCurrentEGLContext(LinuxWaylandGLSwapChainContext* context);

    private:

        EGLDisplay    dpy_ = nullptr;
        EGLSurface    wnd_ = nullptr;
        EGLContext    glc_ = nullptr;

};

} // /namespace LLGL

#endif // LLGL_LINUX_ENABLE_WAYLAND

#endif // LLGL_LINUX_GL_SWAP_CHAIN_CONTEXT_WAYLAND_H

// ================================================================================