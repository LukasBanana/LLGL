/*
 * DeprecatedFallback.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_C99_DEPRECATED_FALLBACL_H
#define LLGL_C99_DEPRECATED_FALLBACL_H


#include <LLGL-C/Canvas.h>

// DEPRECATED

inline void llglPostCanvasTapGesture_DEPRECATED(LLGLCanvas sender, const LLGLOffset2D* position, uint32_t numTouches)
{
    llglPostCanvasTapGesture(sender, *position, 0, numTouches);
}

inline void llglPostCanvasPanGesture_DEPRECATED(LLGLCanvas sender, const LLGLOffset2D* position, uint32_t numTouches, float dX, float dY, LLGLEventAction action)
{
    llglPostCanvasPanGesture(sender, *position, 0, numTouches, dX, dY, action);
}

#define llglPostCanvasTapGesture(...) llglPostCanvasTapGesture_DEPRECATED(__VA_ARGS__)
#define llglPostCanvasPanGesture(...) llglPostCanvasPanGesture_DEPRECATED(__VA_ARGS__)

// /DEPRECATED


#endif



// ================================================================================
