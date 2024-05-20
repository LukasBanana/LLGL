/*
 * UWPPath.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "../Path.h"
#include <Windows.h>


namespace LLGL
{

namespace Path
{


LLGL_EXPORT char GetSeparator()
{
    return '\\';
}

LLGL_EXPORT UTF8String GetWorkingDir()
{
    const DWORD pathLen = ::GetCurrentDirectory(0, nullptr);
    if (pathLen > 0)
    {
        /* Override content of string including the NUL-terminator, which is allowed since C++11 */
        #ifdef UNICODE
        std::wstring path;
        #else
        std::string path;
        #endif
        path.resize(static_cast<std::size_t>(pathLen - 1));
        ::GetCurrentDirectory(pathLen, &path[0]);
        return path;
    }
    return "";
}

LLGL_EXPORT UTF8String GetAbsolutePath(const UTF8String& filename)
{
    return Combine(GetWorkingDir(), filename);
}


} // /nameapace Path

} // /namespace LLGL



// ================================================================================
