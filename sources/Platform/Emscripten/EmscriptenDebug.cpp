/*
 * LinuxDebug.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "../Debug.h"
#include <stdio.h>


namespace LLGL
{


LLGL_EXPORT void DebugPuts(const char* text)
{
    ::fprintf(stderr, "%s\n", text);
}


} // /namespace LLGL



// ================================================================================
