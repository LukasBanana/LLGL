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
typedef void (*LLGL_PFN_OnCanvasQuit)(LLGLCanvas sender, bool* veto); //LLGL_DEPRECATED("LLGL_PFN_OnCanvasQuit is deprecated since 0.04b; Use custom state instead!")
typedef void (*LLGL_PFN_OnCanvasInit)(LLGLCanvas sender);
typedef void (*LLGL_PFN_OnCanvasDestroy)(LLGLCanvas sender);
typedef void (*LLGL_PFN_OnCanvasDraw)(LLGLCanvas sender);
typedef void (*LLGL_PFN_OnCanvasResize)(LLGLCanvas sender, const LLGLExtent2D* clientAreaSize);
typedef void (*LLGL_PFN_OnCanvasTapGesture)(LLGLCanvas sender, const LLGLOffset2D* position, uint32_t numTouches);
typedef void (*LLGL_PFN_OnCanvasPanGesture)(LLGLCanvas sender, const LLGLOffset2D* position, uint32_t numTouches, float dx, float dy, LLGLEventAction action);
typedef void (*LLGL_PFN_OnCanvasKeyDown)(LLGLCanvas sender, LLGLKey keyCode);
typedef void (*LLGL_PFN_OnCanvasKeyUp)(LLGLCanvas sender, LLGLKey keyCode);

typedef struct LLGLCanvasEventListener
{
    LLGL_PFN_OnCanvasProcessEvents  onProcessEvents;
    LLGL_PFN_OnCanvasQuit           onQuit; //LLGL_DEPRECATED("onQuit is deprecated since 0.04b; Use custom state instead!")
    LLGL_PFN_OnCanvasInit           onInit;
    LLGL_PFN_OnCanvasDestroy        onDestroy;
    LLGL_PFN_OnCanvasDraw           onDraw;
    LLGL_PFN_OnCanvasResize         onResize;
    LLGL_PFN_OnCanvasTapGesture     onTapGesture;
    LLGL_PFN_OnCanvasPanGesture     onPanGesture;
    LLGL_PFN_OnCanvasKeyDown        onKeyDown;
    LLGL_PFN_OnCanvasKeyUp          onKeyUp;
}
LLGLCanvasEventListener;

LLGL_C_EXPORT LLGLCanvas llglCreateCanvas(const LLGLCanvasDescriptor* canvasDesc);
LLGL_C_EXPORT void llglReleaseCanvas(LLGLCanvas canvas);
LLGL_C_EXPORT void llglSetCanvasTitle(LLGLCanvas canvas, const wchar_t* title);
LLGL_C_EXPORT void llglSetCanvasTitleUTF8(LLGLCanvas canvas, const char* title);
LLGL_C_EXPORT size_t llglGetCanvasTitle(LLGLCanvas canvas, size_t outTitleLength, wchar_t* outTitle LLGL_ANNOTATE(NULL));
LLGL_C_EXPORT size_t llglGetCanvasTitleUTF8(LLGLCanvas canvas, size_t outTitleLength, char* outTitle LLGL_ANNOTATE(NULL));
//LLGL_DEPRECATED("llglHasCanvasQuit is deprecated since 0.04b; Use custom state instead!")
LLGL_C_EXPORT bool llglHasCanvasQuit(LLGLCanvas canvas);
LLGL_C_EXPORT void llglSetCanvasUserData(LLGLCanvas canvas, void* userData);
LLGL_C_EXPORT void* llglGetCanvasUserData(LLGLCanvas canvas);
LLGL_C_EXPORT int llglAddCanvasEventListener(LLGLCanvas canvas, const LLGLCanvasEventListener* eventListener);
LLGL_C_EXPORT void llglRemoveCanvasEventListener(LLGLCanvas canvas, int eventListenerID);
//LLGL_DEPRECATED("llglPostCanvasQuit is deprecated since 0.04b; Use custom state instead!")
LLGL_C_EXPORT void llglPostCanvasQuit(LLGLCanvas canvas);
LLGL_C_EXPORT void llglPostCanvasInit(LLGLCanvas sender);
LLGL_C_EXPORT void llglPostCanvasDestroy(LLGLCanvas sender);
LLGL_C_EXPORT void llglPostCanvasDraw(LLGLCanvas sender);
LLGL_C_EXPORT void llglPostCanvasResize(LLGLCanvas sender, const LLGLExtent2D* clientAreaSize);
LLGL_C_EXPORT void llglPostCanvasTapGesture(LLGLCanvas sender, const LLGLOffset2D* position, uint32_t numTouches);
LLGL_C_EXPORT void llglPostCanvasPanGesture(LLGLCanvas sender, const LLGLOffset2D* position, uint32_t numTouches, float dx, float dy, LLGLEventAction action);
LLGL_C_EXPORT void llglPostCanvasKeyDown(LLGLCanvas sender, LLGLKey keyCode);
LLGL_C_EXPORT void llglPostCanvasKeyUp(LLGLCanvas sender, LLGLKey keyCode);


#endif



// ================================================================================
