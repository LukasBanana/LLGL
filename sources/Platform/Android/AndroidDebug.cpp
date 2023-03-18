/*
 * AndroidDebug.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
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
