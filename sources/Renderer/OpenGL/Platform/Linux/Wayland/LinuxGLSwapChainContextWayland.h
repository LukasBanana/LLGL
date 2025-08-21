/*
 * LinuxGLSwapChainContextWayland.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_LINUX_GL_SWAP_CHAIN_CONTEXT_WAYLAND_H
#define LLGL_LINUX_GL_SWAP_CHAIN_CONTEXT_WAYLAND_H


#include "LinuxGLContextWayland.h"
#include "../../GLSwapChainContext.h"

#include <wayland-client.h>

#include "LinuxSharedEGLSurface.h"

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

        LinuxSharedEGLSurfacePtr sharedSurface_;
        EGLDisplay display_ = nullptr;
        EGLContext context_ = nullptr;

};


} // /namespace LLGL



#endif



// ================================================================================