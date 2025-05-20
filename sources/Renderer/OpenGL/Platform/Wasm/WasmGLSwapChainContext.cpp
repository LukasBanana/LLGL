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

WasmGLSwapChainContext::WasmGLSwapChainContext(WasmGLContext& context, Surface& /*surface*/) :
    GLSwapChainContext      { context                          },
    webGLContextHandle_     { context.GetWebGLContext()        },
    hasExplicitSwapControl_ { context.HasExplicitSwapControl() }
{
	LLGL_ASSERT(webGLContextHandle_ != 0);
}

WasmGLSwapChainContext::~WasmGLSwapChainContext()
{
    // dummy
}

bool WasmGLSwapChainContext::HasDrawable() const
{
    return true; // dummy
}

bool WasmGLSwapChainContext::SwapBuffers()
{
    /* Nothing to do, the browser will handle this since we didn't request 'explicitSwapControl' */
    /* Commit frame if explicit swap control was enabled for this WebGL context */
    if (hasExplicitSwapControl_)
        emscripten_webgl_commit_frame();
    return true;
}

void WasmGLSwapChainContext::Resize(const Extent2D& resolution)
{
    // do nothing (WebGL context does not need to be resized)
}

bool WasmGLSwapChainContext::MakeCurrentEGLContext(WasmGLSwapChainContext* context)
{
    EMSCRIPTEN_RESULT result = emscripten_webgl_make_context_current(context->webGLContextHandle_);
    LLGL_ASSERT(result == EMSCRIPTEN_RESULT_SUCCESS, "emscripten_webgl_make_context_current() failed");
    return true;
}


} // /namespace LLGL



// ================================================================================
