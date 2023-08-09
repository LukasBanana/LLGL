/*
 * SwapChain.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_C99_SWAP_CHAIN_H
#define LLGL_C99_SWAP_CHAIN_H


#include <LLGL-C/Export.h>
#include <LLGL-C/LLGLWrapper.h>
#include <stdint.h>


LLGL_C_EXPORT void llglPresent(LLGLSwapChain swapChain);
LLGL_C_EXPORT uint32_t llglGetCurrentSwapIndex(LLGLSwapChain swapChain);
LLGL_C_EXPORT uint32_t llglGetNumSwapBuffers(LLGLSwapChain swapChain);
LLGL_C_EXPORT LLGLFormat llglGetColorFormat(LLGLSwapChain swapChain);
LLGL_C_EXPORT LLGLFormat llglGetDepthStencilFormat(LLGLSwapChain swapChain);
LLGL_C_EXPORT bool llglResizeBuffers(LLGLSwapChain swapChain, const LLGLExtent2D* resolution, long flags);
LLGL_C_EXPORT bool llglSetVsyncInterval(LLGLSwapChain swapChain, uint32_t vsyncInterval);
LLGL_C_EXPORT bool llglSwitchFullscreen(LLGLSwapChain swapChain, bool enable);
LLGL_C_EXPORT LLGLSurface llglGetSurface(LLGLSwapChain swapChain);


#endif



// ================================================================================
