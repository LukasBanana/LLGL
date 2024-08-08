/*
 * Win32GLSwapChainContext.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_WIN32_GL_SWAP_CHAIN_CONTEXT_H
#define LLGL_WIN32_GL_SWAP_CHAIN_CONTEXT_H


#include "../GLSwapChainContext.h"
#include "../../OpenGL.h"


namespace LLGL
{


class Surface;
class Win32GLContext;

class Win32GLSwapChainContext final : public GLSwapChainContext
{

    public:

        Win32GLSwapChainContext(Win32GLContext& context, Surface& surface);

        bool HasDrawable() const override;
        bool SwapBuffers() override;
        void Resize(const Extent2D& resolution) override;

    public:

        static bool MakeCurrentWGLContext(Win32GLSwapChainContext* context);

    private:

        HGLRC   hGLRC_  = nullptr;
        HDC     hDC_    = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
