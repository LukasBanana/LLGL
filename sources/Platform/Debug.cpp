/*
 * Debug.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "Debug.h"
#include "../Core/CoreUtils.h"
#include <stdio.h>
#include <stdarg.h>


namespace LLGL
{


LLGL_EXPORT void DebugPrintf(const char* format, ...)
{
    va_list args;
    va_start(args, format);

    int len = ::vsnprintf(nullptr, 0, format, args);
    if (len > 0)
    {
        const auto formatLen = static_cast<std::size_t>(len);
        auto formatStr = MakeUniqueArray<char>(formatLen + 1);
        ::vsnprintf(formatStr.get(), formatLen + 1, format, args);
        DebugPuts(formatStr.get());
    }

    va_end(args);
}


} // /namespace LLGL



// ================================================================================
