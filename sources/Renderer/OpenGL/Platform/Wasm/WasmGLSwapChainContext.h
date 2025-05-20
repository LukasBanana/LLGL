/*
 * WasmGLSwapChainContext.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_WASM_GL_SWAP_CHAIN_CONTEXT_H
#define LLGL_WASM_GL_SWAP_CHAIN_CONTEXT_H


#include "../GLSwapChainContext.h"
#include "../../OpenGL.h"
#include "WasmGLContext.h"

namespace LLGL
{


class Surface;
class WasmGLContext;

class WasmGLSwapChainContext final : public GLSwapChainContext
{

    public:

        WasmGLSwapChainContext(WasmGLContext& context, Surface& surface);
        ~WasmGLSwapChainContext();

        bool HasDrawable() const override;
        bool SwapBuffers() override;
        void Resize(const Extent2D& resolution) override;

    public:

        static bool MakeCurrentEGLContext(WasmGLSwapChainContext* context);

    private:

        EMSCRIPTEN_WEBGL_CONTEXT_HANDLE webGLContextHandle_     = 0;
        bool                            hasExplicitSwapControl_ = false;

};


} // /namespace LLGL


#endif



// ================================================================================
