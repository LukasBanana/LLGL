/*
 * Win32Debug.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "../Debug.h"
#include <stdio.h>

#ifdef LLGL_DEBUG
#   include <Windows.h>
#endif


namespace LLGL
{


LLGL_EXPORT void DebugPuts(const char* text)
{
    #ifdef LLGL_DEBUG
    if (IsDebuggerPresent())
    {
        /* Print to Visual Debugger */
        OutputDebugStringA(text);
        OutputDebugStringA("\n");
    }
    else
    #endif
    {
        /* Print to standard error stream */
        ::fprintf(stderr, "%s\n", text);
    }
}


} // /namespace LLGL



// ================================================================================
