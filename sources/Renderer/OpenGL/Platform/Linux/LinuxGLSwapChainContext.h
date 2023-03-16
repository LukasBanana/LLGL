/*
 * LinuxGLSwapChainContext.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_LINUX_GL_SWAP_CHAIN_CONTEXT_H
#define LLGL_LINUX_GL_SWAP_CHAIN_CONTEXT_H


#include "../GLSwapChainContext.h"
#include "../../OpenGL.h"
#include <X11/Xlib.h>


namespace LLGL
{


class Surface;
class LinuxGLContext;

class LinuxGLSwapChainContext final : public GLSwapChainContext
{

    public:

        LinuxGLSwapChainContext(LinuxGLContext& context, Surface& surface);

        bool SwapBuffers() override;

    public:

        static bool MakeCurrentGLXContext(LinuxGLSwapChainContext* context);

    private:

        ::Display*      dpy_ = nullptr;
        ::Window        wnd_ = 0;
        ::GLXContext    glc_ = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
