/*
 * Display.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_C99_DISPLAY_H
#define LLGL_C99_DISPLAY_H


#include <LLGL-C/Export.h>
#include <LLGL-C/LLGLWrapper.h>
#include <stddef.h>
#include <stdbool.h>


LLGL_C_EXPORT size_t llglDisplayCount();
LLGL_C_EXPORT LLGLDisplay const * llglGetDisplayList();
LLGL_C_EXPORT LLGLDisplay llglGetDisplay(size_t index);
LLGL_C_EXPORT LLGLDisplay llglGetPrimaryDisplay();
LLGL_C_EXPORT bool llglShowCursor(bool show);
LLGL_C_EXPORT bool llglIsCursorShown();
LLGL_C_EXPORT bool llglSetCursorPosition(const LLGLOffset2D* position);
LLGL_C_EXPORT void llglGetCursorPosition(LLGLOffset2D* outPosition);

LLGL_C_EXPORT bool llglIsDisplayPrimary(LLGLDisplay display);
LLGL_C_EXPORT size_t llglGetDisplayDeviceName(LLGLDisplay display, size_t outNameLength, wchar_t* outName LLGL_ANNOTATE(NULL, [outNameLength]));
LLGL_C_EXPORT void llglGetDisplayOffset(LLGLDisplay display, LLGLOffset2D* outOffset);
LLGL_C_EXPORT bool llglResetDisplayMode(LLGLDisplay display);
LLGL_C_EXPORT bool llglSetDisplayMode(LLGLDisplay display, const LLGLDisplayMode* displayMode);
LLGL_C_EXPORT void llglGetDisplayMode(LLGLDisplay display, LLGLDisplayMode* outDisplayMode);
LLGL_C_EXPORT size_t llglGetSupportedDisplayModes(LLGLDisplay display, size_t maxNumDisplayModes, LLGLDisplayMode* outDisplayModes LLGL_ANNOTATE(NULL, [maxNumDisplayModes]));


#endif



// ================================================================================
