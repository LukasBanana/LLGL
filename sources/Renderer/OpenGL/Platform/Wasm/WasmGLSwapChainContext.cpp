/*
 * WasmGLSwapChainContext.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "WasmGLSwapChainContext.h"
#include "../../../../Core/CoreUtils.h"
#include "../../../../Core/Assertion.h"
#include <LLGL/Platform/NativeHandle.h>


namespace LLGL
{


/*
 * GLSwapChainContext class
 */

std::unique_ptr<GLSwapChainContext> GLSwapChainContext::Create(GLContext& context, Surface& surface)
{
    return MakeUnique<WasmGLSwapChainContext>(static_cast<WasmGLContext&>(context), surface);
}

bool GLSwapChainContext::MakeCurrentUnchecked(GLSwapChainContext* context)
{
    return WasmGLSwapChainContext::MakeCurrentEGLContext(static_cast<WasmGLSwapChainContext*>(context));
}


/*
 * WasmGLSwapChainContext class
 */

WasmGLSwapChainContext::WasmGLSwapChainContext(WasmGLContext& context, Surface& surface) :
    GLSwapChainContext { context                   },
    context_           { context.GetWebGLContext() }
{
    /* Get native surface handle */
    //NativeHandle nativeHandle;
    //surface.GetNativeHandle(&nativeHandle, sizeof(nativeHandle));
	
	if (!context_)
        LLGL_TRAP("GetWebGLContext failed");
}

WasmGLSwapChainContext::~WasmGLSwapChainContext()
{
    //eglDestroySurface(display_, surface_);
}

bool WasmGLSwapChainContext::HasDrawable() const
{
    return true; // dummy
}

bool WasmGLSwapChainContext::SwapBuffers()
{
    // do nothing
    return true;
}

void WasmGLSwapChainContext::Resize(const Extent2D& resolution)
{
    // do nothing (WebGL context does not need to be resized)
}

bool WasmGLSwapChainContext::MakeCurrentEGLContext(WasmGLSwapChainContext* context)
{
    EMSCRIPTEN_RESULT res = emscripten_webgl_make_context_current(context->context_);
    LLGL_ASSERT(res == EMSCRIPTEN_RESULT_SUCCESS, "emscripten_webgl_make_context_current() failed");

    //LLGL_ASSERT(emscripten_webgl_get_current_context() == context->GetGLContext());

    //int width = 0, height = 0, fs = 0;
    //emscripten_webgl_get_drawing_buffer_size(context->context_, &width, &height);

    return true;
}


} // /namespace LLGL



// ================================================================================
