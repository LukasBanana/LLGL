/*
 * C99SwapChain.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/SwapChain.h>
#include <LLGL-C/SwapChain.h>
#include "C99Internal.h"


// namespace LLGL {


using namespace LLGL;

LLGL_C_EXPORT bool llglIsPresentable(LLGLSwapChain swapChain)
{
    return LLGL_PTR(SwapChain, swapChain)->IsPresentable();
}

LLGL_C_EXPORT void llglPresent(LLGLSwapChain swapChain)
{
    LLGL_PTR(SwapChain, swapChain)->Present();
}

LLGL_C_EXPORT uint32_t llglGetCurrentSwapIndex(LLGLSwapChain swapChain)
{
    return LLGL_PTR(SwapChain, swapChain)->GetCurrentSwapIndex();
}

LLGL_C_EXPORT uint32_t llglGetNumSwapBuffers(LLGLSwapChain swapChain)
{
    return LLGL_PTR(SwapChain, swapChain)->GetNumSwapBuffers();
}

LLGL_C_EXPORT LLGLFormat llglGetColorFormat(LLGLSwapChain swapChain)
{
    return (LLGLFormat)LLGL_PTR(SwapChain, swapChain)->GetColorFormat();
}

LLGL_C_EXPORT LLGLFormat llglGetDepthStencilFormat(LLGLSwapChain swapChain)
{
    return (LLGLFormat)LLGL_PTR(SwapChain, swapChain)->GetDepthStencilFormat();
}

LLGL_C_EXPORT bool llglResizeBuffers(LLGLSwapChain swapChain, const LLGLExtent2D* resolution, long flags)
{
    return LLGL_PTR(SwapChain, swapChain)->ResizeBuffers(*(const Extent2D*)resolution, flags);
}

LLGL_C_EXPORT bool llglSetVsyncInterval(LLGLSwapChain swapChain, uint32_t vsyncInterval)
{
    return LLGL_PTR(SwapChain, swapChain)->SetVsyncInterval(vsyncInterval);
}

LLGL_C_EXPORT bool llglSwitchFullscreen(LLGLSwapChain swapChain, bool enable)
{
    return LLGL_PTR(SwapChain, swapChain)->SwitchFullscreen(enable);
}

LLGL_C_EXPORT LLGLSurface llglGetSurface(LLGLSwapChain swapChain)
{
    return LLGLSurface{ &(LLGL_PTR(SwapChain, swapChain)->GetSurface()) };
}


// } /namespace LLGL



// ================================================================================
