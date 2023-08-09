/*
 * Canvas.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_C99_CANVAS_H
#define LLGL_C99_CANVAS_H


#include <LLGL-C/Export.h>
#include <LLGL-C/LLGLWrapper.h>
#include <LLGL-C/Surface.h>


typedef void (*LLGL_PFN_OnCanvasProcessEvents)(LLGLCanvas sender);
typedef void (*LLGL_PFN_OnCanvasQuit)(LLGLCanvas sender, bool* veto);

typedef struct LLGLCanvasEventListener
{
    LLGL_PFN_OnCanvasProcessEvents  onProcessEvents;
    LLGL_PFN_OnCanvasQuit           onQuit;
}
LLGLCanvasEventListener;

LLGL_C_EXPORT LLGLCanvas llglCreateCanvas(const LLGLCanvasDescriptor* canvasDesc);
LLGL_C_EXPORT void llglReleaseCanvas(LLGLCanvas canvas);
LLGL_C_EXPORT void llglSetCanvasTitle(LLGLCanvas canvas, const wchar_t* title);
LLGL_C_EXPORT size_t llglGetCanvasTitle(LLGLCanvas canvas, size_t outTitleLength, wchar_t* LLGL_NULLABLE(outTitle));
LLGL_C_EXPORT bool llglHasCanvasQuit(LLGLCanvas canvas);
LLGL_C_EXPORT int llglAddCanvasEventListener(LLGLCanvas canvas, const LLGLCanvasEventListener* eventListener);
LLGL_C_EXPORT void llglRemoveCanvasEventListener(LLGLCanvas canvas, int eventListenerID);
LLGL_C_EXPORT void llglPostCanvasQuit(LLGLCanvas canvas);


#endif



// ================================================================================
