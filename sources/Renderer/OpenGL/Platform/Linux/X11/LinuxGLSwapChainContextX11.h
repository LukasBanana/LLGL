/*
 * LinuxGLSwapChainContextX11.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_LINUX_GL_SWAP_CHAIN_CONTEXT_X11_H
#define LLGL_LINUX_GL_SWAP_CHAIN_CONTEXT_X11_H


#include "LinuxGLContextX11.h"
#include "../../GLSwapChainContext.h"
#include <X11/Xlib.h>

namespace LLGL
{


class Surface;
class LinuxGLContext;

class LinuxGLSwapChainContextX11 final : public GLSwapChainContext
{

    public:

        LinuxGLSwapChainContextX11(LinuxGLContextX11& context, Surface& surface);

        bool HasDrawable() const override;
        bool SwapBuffers() override;
        void Resize(const Extent2D& resolution) override;

    public:

        static bool MakeCurrentGLXContext(LinuxGLSwapChainContextX11* context);

    private:

        ::Display*      dpy_ = nullptr;
        ::Window        wnd_ = 0;
        ::GLXContext    glc_ = nullptr;

};

} // /namespace LLGL

#endif



// ================================================================================
