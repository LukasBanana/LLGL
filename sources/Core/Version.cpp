/*
 * Version.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/Version.h>
#include "VersionMacros.h"
#include <string>


namespace LLGL
{

namespace Version
{


LLGL_EXPORT unsigned GetMajor()
{
    return LLGL_VERSION_MAJOR;
}

LLGL_EXPORT unsigned GetMinor()
{
    return LLGL_VERSION_MINOR;
}

LLGL_EXPORT unsigned GetRevision()
{
    return LLGL_VERSION_REVISION;
}

LLGL_EXPORT const char* GetStatus()
{
    return LLGL_VERSION_STATUS;
}

LLGL_EXPORT unsigned GetID()
{
    return LLGL_VERSION_ID;
}

static std::string BuildVersionString()
{
    std::string s;

    s += std::to_string(GetMajor());
    s += '.';
    if (GetMinor() < 10)
        s += '0';
    s += std::to_string(GetMinor());

    if (GetStatus() && *GetStatus() != '\0')
    {
        s += ' ';
        s += GetStatus();
    }

    if (GetRevision())
    {
        s += " (Rev. ";
        s += std::to_string(GetRevision());
        s += ')';
    }

    return s;
}

LLGL_EXPORT const char* GetString()
{
    static std::string s = BuildVersionString();
    return s.c_str();
}


} // /namespace Version

} // /namespace LLGL



// ================================================================================
