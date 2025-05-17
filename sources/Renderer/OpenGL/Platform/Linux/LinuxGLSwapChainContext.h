/*
 * LinuxGLSwapChainContext.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_LINUX_GL_SWAP_CHAIN_CONTEXT_H
#define LLGL_LINUX_GL_SWAP_CHAIN_CONTEXT_H


#include "../GLSwapChainContext.h"
#include "../../OpenGL.h"
#include <X11/Xlib.h>

#ifdef LLGL_LINUX_ENABLE_WAYLAND
#include <wayland-client.h>
#endif

namespace LLGL
{


class Surface;
class LinuxGLContext;

class LinuxX11GLSwapChainContext final : public GLSwapChainContext
{

    public:

        LinuxX11GLSwapChainContext(LinuxGLContext& context, Surface& surface);

        bool HasDrawable() const override;
        bool SwapBuffers() override;
        void Resize(const Extent2D& resolution) override;

    public:

        static bool MakeCurrentGLXContext(LinuxX11GLSwapChainContext* context);

    private:

        ::Display*      dpy_ = nullptr;
        ::Window        wnd_ = 0;
        ::GLXContext    glc_ = nullptr;

};

#ifdef LLGL_LINUX_ENABLE_WAYLAND

class LinuxWaylandGLSwapChainContext final : public GLSwapChainContext
{

    public:

        LinuxWaylandGLSwapChainContext(LinuxGLContext& context, Surface& surface);

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

#endif

} // /namespace LLGL


#endif



// ================================================================================
