/*
 * Debug.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_DEBUG_H
#define LLGL_DEBUG_H


#include <LLGL/Export.h>


namespace LLGL
{


// Prints the specified text to the platform specific debug output or the standard error stream (stderr) by default.
LLGL_EXPORT void DebugPuts(const char* text);

// Prints the specified formatted text to the debug output. Calls DebugPuts with the formatted string.
LLGL_EXPORT void DebugPrintf(const char* format, ...);


} // /namespace LLGL


#endif



// ================================================================================
