/*
 * TestbedUtils.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "TestbedUtils.h"
#include <string.h>


static bool StringEndsWithPrimary(const char* str, std::size_t strLen, const char* end, std::size_t endLen)
{
    return (strLen >= endLen && ::strncmp(str + strLen - endLen, end, endLen) == 0);
}

bool StringEndsWith(const char* str, const char* end)
{
    return StringEndsWithPrimary(str, ::strlen(str), end, ::strlen(end));
}

bool StringEndsWith(const std::string& str, const std::string& end)
{
    return StringEndsWithPrimary(str.c_str(), str.size(), end.c_str(), end.size());
}


