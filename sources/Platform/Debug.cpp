/*
 * Debug.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "Debug.h"
#include <stdio.h>
#include <stdarg.h>
#include <string>


namespace LLGL
{


LLGL_EXPORT void DebugPrintf(const char* format, ...)
{
    va_list args;
    va_start(args, format);

    int len = ::snprintf(nullptr, 0, format, args);
    if (len > 0)
    {
        std::string text;
        text.resize(static_cast<std::size_t>(len));
        ::snprintf(&text[0], sizeof(text), format, args);
        DebugPuts(text.c_str());
    }

    va_end(args);
}


} // /namespace LLGL



// ================================================================================
