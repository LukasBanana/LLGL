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


namespace LLGL
{


class LinuxGLSwapChainContextWayland final : public GLSwapChainContext
{

    public:

        LinuxGLSwapChainContextWayland(LinuxGLContextWayland& context, Surface& surface);

        bool HasDrawable() const override;
        bool SwapBuffers() override;
        void Resize(const Extent2D& resolution) override;

    public:

        static bool MakeCurrentEGLContext(LinuxGLSwapChainContextWayland* context);

    private:

        EGLDisplay display_ = nullptr;
        EGLSurface surface_ = nullptr;
        EGLContext context_ = nullptr;

};


} // /namespace LLGL

#endif // LLGL_LINUX_ENABLE_WAYLAND


#endif



// ================================================================================