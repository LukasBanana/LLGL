/*
 * C99Surface.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/Surface.h>
#include <LLGL-C/Surface.h>
#include "C99Internal.h"


// namespace LLGL {


using namespace LLGL;

LLGL_C_EXPORT bool llglGetSurfaceNativeHandle(LLGLSurface surface, void* nativeHandle, size_t nativeHandleSize)
{
    return LLGL_PTR(Surface, surface)->GetNativeHandle(nativeHandle, nativeHandleSize);
}

LLGL_C_EXPORT void llglGetSurfaceContentSize(LLGLSurface surface, LLGLExtent2D* outSize)
{
    Extent2D internalSize = LLGL_PTR(Surface, surface)->GetContentSize();
    outSize->width = internalSize.width;
    outSize->height = internalSize.height;
}

LLGL_C_EXPORT bool llglAdaptSurfaceForVideoMode(LLGLSurface surface, LLGLExtent2D* outResolution, bool* outFullscreen)
{
    return LLGL_PTR(Surface, surface)->AdaptForVideoMode((Extent2D*)outResolution, outFullscreen);
}

LLGL_C_EXPORT void llglResetSurfacePixelFormat(LLGLSurface surface)
{
    // dummy
}

LLGL_C_EXPORT bool llglProcessSurfaceEvents()
{
    return LLGL::Surface::ProcessEvents();
}

LLGL_C_EXPORT LLGLDisplay llglFindSurfaceResidentDisplay(LLGLSurface surface)
{
    return LLGLDisplay{ LLGL_PTR(Surface, surface)->FindResidentDisplay() };
}


// } /namespace LLGL



// ================================================================================
