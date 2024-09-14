/*
 * WasmGLContext.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_WASM_CONTEXT_H
#define LLGL_WASM_CONTEXT_H


#include "../GLContext.h"
#include "../../OpenGL.h"
#include <emscripten.h>
#include <emscripten/html5.h>

namespace LLGL
{


// Implementation of the <GLContext> interface for Android and wrapper for a native EGL context.
class WasmGLContext : public GLContext
{

    public:

        WasmGLContext(
            const GLPixelFormat&                pixelFormat,
            const RendererConfigurationOpenGL&  profile,
            Surface&                            surface,
            WasmGLContext*                      sharedContext
        );
        ~WasmGLContext();

        int GetSamples() const override;

        bool GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize) const override;

    public:

        // Returns the native WebGL context.
        inline EMSCRIPTEN_WEBGL_CONTEXT_HANDLE GetWebGLContext() const
        {
            return webGLContextHandle_;
        }

        // Returns true if this context enabled explicit swap control.
        inline bool HasExplicitSwapControl() const
        {
            return hasExplicitSwapControl_;
        }

    private:
    
        bool SetSwapInterval(int interval) override;

        void CreateContext(
            const GLPixelFormat&                pixelFormat,
            const RendererConfigurationOpenGL&  profile,
            WasmGLContext*                      sharedContext
        );
        void DeleteContext();

    private:

        EMSCRIPTEN_WEBGL_CONTEXT_HANDLE webGLContextHandle_     = 0;
        int                             samples_                = 0;
        bool                            hasExplicitSwapControl_ = false;

};


} // /namespace LLGL


#endif



// ================================================================================
