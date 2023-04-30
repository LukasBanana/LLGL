/*
 * Debug.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "Debug.h"
#include "../Core/CoreUtils.h"
#include "../Core/StringUtils.h"
#include <stdio.h>
#include <stdarg.h>


namespace LLGL
{


LLGL_EXPORT void DebugPrintf(const char* format, ...)
{
    std::string str;
    LLGL_STRING_PRINTF(str, format);
    DebugPuts(str.c_str());
}


} // /namespace LLGL



// ================================================================================
