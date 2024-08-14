/*
 * EmscriptenGLSwapChainContext.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_EMSCRIPTEN_GL_SWAP_CHAIN_CONTEXT_H
#define LLGL_EMSCRIPTEN_GL_SWAP_CHAIN_CONTEXT_H


#include "../GLSwapChainContext.h"
#include "../../OpenGL.h"
#include "EmscriptenGLContext.h"

namespace LLGL
{


class Surface;
class EmscriptenGLContext;

class EmscriptenGLSwapChainContext final : public GLSwapChainContext
{

    public:

        EmscriptenGLSwapChainContext(EmscriptenGLContext& context, Surface& surface);
        ~EmscriptenGLSwapChainContext();

        bool HasDrawable() const override;
        bool SwapBuffers() override;
        void Resize(const Extent2D& resolution) override;

    public:

        static bool MakeCurrentEGLContext(EmscriptenGLSwapChainContext* context);

    private:

        EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context_ = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
