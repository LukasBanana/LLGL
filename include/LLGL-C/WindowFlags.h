/*
 * WindowFlags.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_C99_WINDOW_FLAGS_H
#define LLGL_C99_WINDOW_FLAGS_H


#include <LLGL-C/Types.h>
#include <stdint.h>
#include <stdbool.h>


typedef struct LLGLWindowDescriptor
{
    const char*     title;
    LLGLOffset2D    position;
    LLGLExtent2D    size;
    bool            visible;
    bool            borderless;
    bool            resizable;
    bool            acceptDropFiles;
    bool            centered;
    const void*     windowContext;
}
LLGLWindowDescriptor;

typedef struct LLGLWindowBehavior
{
    bool        disableClearOnResize;
    uint32_t    moveAndResizeTimerID;
}
LLGLWindowBehavior;


#endif



// ================================================================================
