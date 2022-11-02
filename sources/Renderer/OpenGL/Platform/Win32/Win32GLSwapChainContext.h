/*
 * Win32GLSwapChainContext.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
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

        bool SwapBuffers() override;

    public:

        static bool MakeCurrentWGLContext(Win32GLSwapChainContext* context);

    private:

        HGLRC   hGLRC_;
        HDC     hDC_;

};


} // /namespace LLGL


#endif



// ================================================================================
