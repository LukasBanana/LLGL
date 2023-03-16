/*
 * AndroidGLSwapChainContext.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_ANDROID_GL_SWAP_CHAIN_CONTEXT_H
#define LLGL_ANDROID_GL_SWAP_CHAIN_CONTEXT_H


#include "../GLSwapChainContext.h"
#include "../../OpenGL.h"
#include <EGL/egl.h>


namespace LLGL
{


class Surface;
class AndroidGLContext;

class AndroidGLSwapChainContext final : public GLSwapChainContext
{

    public:

        AndroidGLSwapChainContext(AndroidGLContext& context, Surface& surface);
        ~AndroidGLSwapChainContext();

        bool SwapBuffers() override;

    public:

        static bool MakeCurrentEGLContext(AndroidGLSwapChainContext* context);

    private:

        ::EGLDisplay display_ = nullptr;
        ::EGLContext context_ = nullptr;
        ::EGLSurface surface_ = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
