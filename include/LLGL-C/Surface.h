/*
 * Surface.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_C99_SURFACE_H
#define LLGL_C99_SURFACE_H


#include <LLGL-C/Export.h>
#include <LLGL-C/LLGLWrapper.h>
#include <stdbool.h>
#include <stddef.h>


LLGL_C_EXPORT bool llglGetSurfaceNativeHandle(LLGLSurface surface, void* nativeHandle, size_t nativeHandleSize);
LLGL_C_EXPORT void llglGetSurfaceContentSize(LLGLSurface surface, LLGLExtent2D* outSize);
LLGL_C_EXPORT bool llglAdaptSurfaceForVideoMode(LLGLSurface surface, LLGLExtent2D* outResolution, bool* outFullscreen);
LLGL_C_EXPORT void llglResetSurfacePixelFormat(LLGLSurface surface);
LLGL_C_EXPORT bool llglProcessSurfaceEvents();
LLGL_C_EXPORT LLGLDisplay llglFindSurfaceResidentDisplay(LLGLSurface surface);


#endif



// ================================================================================
