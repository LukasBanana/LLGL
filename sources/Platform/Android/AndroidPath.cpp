/*
 * AndroidPath.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "../Path.h"
#include <unistd.h>


namespace LLGL
{

namespace Path
{


LLGL_EXPORT char GetSeparator()
{
    return '/';
}

LLGL_EXPORT UTF8String GetWorkingDir()
{
    char path[4096] = { 0 };
    return UTF8String{ ::getcwd(path, sizeof(path)) };
}

LLGL_EXPORT UTF8String GetAbsolutePath(const UTF8String& filename)
{
    return Combine(GetWorkingDir(), filename);
}


} // /nameapace Path

} // /namespace LLGL



// ================================================================================
