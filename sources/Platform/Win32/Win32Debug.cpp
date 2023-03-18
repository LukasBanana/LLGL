/*
 * Win32Debug.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
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
