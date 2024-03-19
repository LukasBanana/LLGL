/*
 * EmscriptenGLSwapChainContext.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "EmscriptenGLSwapChainContext.h"
#include "../../../../Core/CoreUtils.h"
#include <LLGL/Platform/NativeHandle.h>

namespace LLGL
{


/*
 * GLSwapChainContext class
 */

std::unique_ptr<GLSwapChainContext> GLSwapChainContext::Create(GLContext& context, Surface& surface)
{
    return MakeUnique<EmscriptenGLSwapChainContext>(static_cast<EmscriptenGLContext&>(context), surface);
}

bool GLSwapChainContext::MakeCurrentUnchecked(GLSwapChainContext* context)
{
    return EmscriptenGLSwapChainContext::MakeCurrentEGLContext(static_cast<EmscriptenGLSwapChainContext*>(context));
}


/*
 * EmscriptenGLSwapChainContext class
 */

EmscriptenGLSwapChainContext::EmscriptenGLSwapChainContext(EmscriptenGLContext& context, Surface& surface) :
    GLSwapChainContext { context }
{
    /* Get native surface handle */
    NativeHandle nativeHandle;
    surface.GetNativeHandle(&nativeHandle, sizeof(nativeHandle));
	
	if (!context.GetWebGLContext())
        throw std::runtime_error("GetWebGLContext failed");
}

EmscriptenGLSwapChainContext::~EmscriptenGLSwapChainContext()
{
    //eglDestroySurface(display_, surface_);
}

bool EmscriptenGLSwapChainContext::SwapBuffers()
{
    // do nothing
    return true;
}

void EmscriptenGLSwapChainContext::Resize(const Extent2D& resolution)
{
    // do nothing (WebGL context does not need to be resized)
}

bool EmscriptenGLSwapChainContext::MakeCurrentEGLContext(EmscriptenGLSwapChainContext* context)
{
    return true;
    EMSCRIPTEN_RESULT res = emscripten_webgl_make_context_current(context->context_);

    if (res == EMSCRIPTEN_RESULT_SUCCESS)
    {
        //assert(emscripten_webgl_get_current_context() == context->GetGLContext());

        int width, height, fs = 0;
        emscripten_get_canvas_element_size("#mycanvas", &width, &height);
        printf("width:%d, height:%d\n", width, height);
        //SetViewportSize((ndf32)width, (ndf32)height); 

        return true;
    }
    else
    {
        throw std::runtime_error("emscripten_webgl_make_context_current failed");
        return false;
    }
}


} // /namespace LLGL



// ================================================================================
