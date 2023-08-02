/*
 * SwapChainFlags.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_C99_SWAP_CHAIN_FLAGS_H
#define LLGL_C99_SWAP_CHAIN_FLAGS_H


#include <stdint.h>
#include <stdbool.h>


/* ----- Flags ----- */

enum LLGLResizeBuffersFlags
{
    LLGLResizeBuffersAdaptSurface    = (1 << 0),
    LLGLResizeBuffersFullscreenMode  = (1 << 1),
    LLGLResizeBuffersWindowedMode    = (1 << 2),
};


/* ----- Structures ----- */

typedef struct LLGLSwapChainDescriptor
{
    LLGLExtent2D    resolution;
    int             colorBits;
    int             depthBits;
    int             stencilBits;
    uint32_t        samples;
    uint32_t        swapBuffers;
    bool            fullscreen;
}
LLGLSwapChainDescriptor;


#endif



// ================================================================================
