/*
 * Debug.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_DEBUG_H
#define LLGL_DEBUG_H


#include <LLGL/Export.h>

#ifdef LLGL_DEBUG
#   include <LLGL/Platform/Platform.h>
#   if defined(LLGL_OS_WIN32)
#      include "Win32/Win32Debug.h"
#   elif defined(LLGL_OS_MACOS)
#      include "MacOS/MacOSDebug.h"
#   elif defined(LLGL_OS_LINUX)
#      include "Linux/LinuxDebug.h"
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


} // /namespace LLGL


#endif



// ================================================================================
