/*
 * Window.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_C99_WINDOW_H
#define LLGL_C99_WINDOW_H


#include <LLGL-C/Export.h>
#include <LLGL-C/LLGLWrapper.h>
#include <stdbool.h>


typedef void (*LLGL_PFN_OnWindowQuit)(LLGLWindow sender, bool* veto);
typedef void (*LLGL_PFN_OnWindowKeyDown)(LLGLWindow sender, LLGLKey keyCode);
typedef void (*LLGL_PFN_OnWindowKeyUp)(LLGLWindow sender, LLGLKey keyCode);
typedef void (*LLGL_PFN_OnWindowDoubleClick)(LLGLWindow sender, LLGLKey keyCode);
typedef void (*LLGL_PFN_OnWindowChar)(LLGLWindow sender, wchar_t chr);
typedef void (*LLGL_PFN_OnWindowWheelMotion)(LLGLWindow sender, int motion);
typedef void (*LLGL_PFN_OnWindowLocalMotion)(LLGLWindow sender, const LLGLOffset2D* position);
typedef void (*LLGL_PFN_OnWindowGlobalMotion)(LLGLWindow sender, const LLGLOffset2D* motion);
typedef void (*LLGL_PFN_OnWindowResize)(LLGLWindow sender, const LLGLExtent2D* clientAreaSize);
typedef void (*LLGL_PFN_OnWindowUpdate)(LLGLWindow sender);
typedef void (*LLGL_PFN_OnWindowGetFocus)(LLGLWindow sender);
typedef void (*LLGL_PFN_OnWindowLostFocus)(LLGLWindow sender);

typedef struct LLGLWindowEventListener
{
    LLGL_PFN_OnWindowQuit           onQuit;
    LLGL_PFN_OnWindowKeyDown        onKeyDown;
    LLGL_PFN_OnWindowKeyUp          onKeyUp;
    LLGL_PFN_OnWindowDoubleClick    onDoubleClick;
    LLGL_PFN_OnWindowChar           onChar;
    LLGL_PFN_OnWindowWheelMotion    onWheelMotion;
    LLGL_PFN_OnWindowLocalMotion    onLocalMotion;
    LLGL_PFN_OnWindowGlobalMotion   onGlobalMotion;
    LLGL_PFN_OnWindowResize         onResize;
    LLGL_PFN_OnWindowUpdate         onUpdate;
    LLGL_PFN_OnWindowGetFocus       onGetFocus;
    LLGL_PFN_OnWindowLostFocus      onLostFocus;
}
LLGLWindowEventListener;

LLGL_C_EXPORT LLGLWindow llglCreateWindow(const LLGLWindowDescriptor* windowDesc);
LLGL_C_EXPORT void llglReleaseWindow(LLGLWindow window);
LLGL_C_EXPORT void llglSetWindowPosition(LLGLWindow window, const LLGLOffset2D* position);
LLGL_C_EXPORT void llglGetWindowPosition(LLGLWindow window, LLGLOffset2D* outPosition);
LLGL_C_EXPORT void llglSetWindowSize(LLGLWindow window, const LLGLExtent2D* size, bool useClientArea);
LLGL_C_EXPORT void llglGetWindowSize(LLGLWindow window, LLGLExtent2D* outSize, bool useClientArea);
LLGL_C_EXPORT void llglSetWindowTitle(LLGLWindow window, const wchar_t* title);
LLGL_C_EXPORT size_t llglGetWindowTitle(LLGLWindow window, size_t outTitleLength, wchar_t* LLGL_NULLABLE(outTitle));
LLGL_C_EXPORT void llglShowWindow(LLGLWindow window, bool show);
LLGL_C_EXPORT bool llglIsWindowShown(LLGLWindow window);
LLGL_C_EXPORT void llglSetWindowDesc(LLGLWindow window, const LLGLWindowDescriptor* windowDesc);
LLGL_C_EXPORT void llglGetWindowDesc(LLGLWindow window, LLGLWindowDescriptor* outWindowDesc);
LLGL_C_EXPORT bool llglHasWindowFocus(LLGLWindow window);
LLGL_C_EXPORT bool llglHasWindowQuit(LLGLWindow window);
LLGL_C_EXPORT int llglAddWindowEventListener(LLGLWindow window, const LLGLWindowEventListener* eventListener);
LLGL_C_EXPORT void llglRemoveWindowEventListener(LLGLWindow window, int eventListenerID);
LLGL_C_EXPORT void llglPostWindowQuit(LLGLWindow window);
LLGL_C_EXPORT void llglPostWindowKeyDown(LLGLWindow window, LLGLKey keyCode);
LLGL_C_EXPORT void llglPostWindowKeyUp(LLGLWindow window, LLGLKey keyCode);
LLGL_C_EXPORT void llglPostWindowDoubleClick(LLGLWindow window, LLGLKey keyCode);
LLGL_C_EXPORT void llglPostWindowChar(LLGLWindow window, wchar_t chr);
LLGL_C_EXPORT void llglPostWindowWheelMotion(LLGLWindow window, int motion);
LLGL_C_EXPORT void llglPostWindowLocalMotion(LLGLWindow window, const LLGLOffset2D* position);
LLGL_C_EXPORT void llglPostWindowGlobalMotion(LLGLWindow window, const LLGLOffset2D* motion);
LLGL_C_EXPORT void llglPostWindowResize(LLGLWindow window, const LLGLExtent2D* clientAreaSize);
LLGL_C_EXPORT void llglPostWindowUpdate(LLGLWindow window);
LLGL_C_EXPORT void llglPostWindowGetFocus(LLGLWindow window);
LLGL_C_EXPORT void llglPostWindowLostFocus(LLGLWindow window);


#endif



// ================================================================================
