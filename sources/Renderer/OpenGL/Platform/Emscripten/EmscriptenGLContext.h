/*
 * EmscriptenGLContext.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_EMSCRIPTEN_CONTEXT_H
#define LLGL_EMSCRIPTEN_CONTEXT_H


#include "../GLContext.h"
#include "../../OpenGL.h"
#include <emscripten.h>
#include <emscripten/html5.h>

namespace LLGL
{


// Implementation of the <GLContext> interface for Android and wrapper for a native EGL context.
class EmscriptenGLContext : public GLContext
{

    public:

        EmscriptenGLContext(
            const GLPixelFormat&                pixelFormat,
            const RendererConfigurationOpenGL&  profile,
            Surface&                            surface,
            EmscriptenGLContext*                sharedContext
        );
        ~EmscriptenGLContext();

        int GetSamples() const override;

        bool GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize) const override;

    public:

        // Returns the native WebGL context.
        inline EMSCRIPTEN_WEBGL_CONTEXT_HANDLE GetWebGLContext() const
        {
            return context_;
        }

    private:
    
        bool SetSwapInterval(int interval) override;

        void CreateContext(
            const GLPixelFormat&                pixelFormat,
            const RendererConfigurationOpenGL&  profile,
            EmscriptenGLContext*                sharedContext
        );
        void DeleteContext();

    private:

        EMSCRIPTEN_WEBGL_CONTEXT_HANDLE context_ = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
