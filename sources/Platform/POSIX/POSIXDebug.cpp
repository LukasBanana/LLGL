/*
 * POSIXDebug.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "../Debug.h"
#include <algorithm>
#include <execinfo.h>
#include <LLGL/Utils/ForRange.h>


namespace LLGL
{


LLGL_EXPORT UTF8String DebugStackTrace(unsigned firstStackFrame, unsigned maxNumStackFrames)
{
    constexpr int bufferSize = 1024;
    void* buffer[bufferSize];

    maxNumStackFrames = std::min(maxNumStackFrames, static_cast<unsigned>(bufferSize));
    if (maxNumStackFrames == 0)
        return "";

    firstStackFrame = std::min(firstStackFrame, maxNumStackFrames - 1);

    /* Get backtrace addresses */
    int count = ::backtrace(buffer, bufferSize);
    if (count <= firstStackFrame)
        return "";

    /* Get backtrace symbols and construct output string */
    UTF8String s;

    char** symbols = ::backtrace_symbols(buffer, count);
    for_subrange_reverse(i, firstStackFrame, std::min(maxNumStackFrames + firstStackFrame, static_cast<unsigned>(count)))
    {
        s += symbols[i];
        s += '\n';
    }

    return s;
}


} // /namespace LLGL



// ================================================================================
