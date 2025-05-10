/*
 * LinuxGLSwapChainContext.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_LINUX_GL_SWAP_CHAIN_CONTEXT_H
#define LLGL_LINUX_GL_SWAP_CHAIN_CONTEXT_H


#include "LinuxGLContextX11.h"
#include "../GLSwapChainContext.h"
#include "../../OpenGL.h"
#include <X11/Xlib.h>
#include <wayland-client.h>

namespace LLGL
{


class Surface;
class LinuxGLContext;

class LinuxX11GLSwapChainContext final : public GLSwapChainContext
{

    public:

        LinuxX11GLSwapChainContext(LinuxGLContextX11& context, Surface& surface);

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

        EGLDisplay dpy_ = nullptr;
        EGLSurface wnd_ = nullptr;
        EGLContext         glc_ = nullptr;

};

} // /namespace LLGL


#endif



// ================================================================================
