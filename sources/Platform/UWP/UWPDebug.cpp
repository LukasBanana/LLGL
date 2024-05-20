/*
 * UWPDebug.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "../Debug.h"
#include "../../Core/StringUtils.h"
#include "UWPModule.h"
#include <stdio.h>
#include <limits.h>
#include <vector>
#include <algorithm>
#include <LLGL/Utils/ForRange.h>

#include <Windows.h>


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

LLGL_EXPORT UTF8String DebugStackTrace(unsigned firstStackFrame, unsigned maxNumStackFrames)
{
    return ""; //todo
}


} // /namespace LLGL



// ================================================================================
