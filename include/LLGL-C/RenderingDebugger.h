/*
 * RenderingDebugger.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_C99_RENDERING_DEBUGGER_H
#define LLGL_C99_RENDERING_DEBUGGER_H


#include <LLGL-C/Export.h>
#include <LLGL-C/LLGLWrapper.h>
#include <stdint.h>
#include <stdbool.h>


LLGL_C_EXPORT LLGLRenderingDebugger llglAllocRenderingDebugger();
LLGL_C_EXPORT void llglFreeRenderingDebugger(LLGLRenderingDebugger debugger);
LLGL_C_EXPORT void llglSetDebuggerTimeRecording(LLGLRenderingDebugger debugger, bool enabled);
LLGL_C_EXPORT bool llglGetDebuggerTimeRecording(LLGLRenderingDebugger debugger);
LLGL_C_EXPORT void llglFlushDebuggerProfile(LLGLRenderingDebugger debugger, LLGLFrameProfile* outFrameProfile);


#endif



// ================================================================================
