/*
 * Debug.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_DEBUG_H
#define LLGL_DEBUG_H


#include <LLGL/Export.h>
#include <LLGL/Container/UTF8String.h>

#ifdef LLGL_DEBUG
#   include <LLGL/Platform/Platform.h>
#   if defined(LLGL_OS_WIN32)
#      include "Win32/Win32Debug.h"
#   elif defined(LLGL_OS_UWP)
#      include "UWP/UWPDebug.h"
#   elif defined(LLGL_OS_MACOS)
#      include "MacOS/MacOSDebug.h"
#   elif defined(LLGL_OS_LINUX)
#      include "Linux/LinuxDebug.h"
#   elif defined(LLGL_OS_WASM)
#      include "Wasm/WasmDebug.h"
#   elif defined(LLGL_OS_IOS)
#      include "IOS/IOSDebug.h"
#   elif defined(LLGL_OS_ANDROID)
#      include "Android/AndroidDebug.h"
#   endif
#else
#   define LLGL_DEBUG_BREAK()
#endif


namespace LLGL
{


// Prints the specified text to the platform specific debug output or the standard error stream (stderr) by default.
LLGL_EXPORT void DebugPuts(const char* text);

// Prints the specified formatted text to the debug output. Calls DebugPuts with the formatted string.
LLGL_EXPORT void DebugPrintf(const char* format, ...);

// Returns a string containing the callstack. The formatting is platform dependent but each line always ends with a newline character '\n'.
LLGL_EXPORT UTF8String DebugStackTrace(unsigned firstStackFrame = 0, unsigned maxNumStackFrames = 64);


} // /namespace LLGL


#endif



// ================================================================================
