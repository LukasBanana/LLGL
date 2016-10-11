/*
 * Version.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/Version.h>
#include "VersionMacros.h"
#include <sstream>


namespace LLGL
{

namespace Version
{


LLGL_EXPORT unsigned int GetMajor()
{
    return LLGL_VERSION_MAJOR;
}

LLGL_EXPORT unsigned int GetMinor()
{
    return LLGL_VERSION_MINOR;
}

LLGL_EXPORT unsigned int GetRevision()
{
    return LLGL_VERSION_REVISION;
}

LLGL_EXPORT std::string GetStatus()
{
    return std::string(LLGL_VERSION_STATUS);
}

LLGL_EXPORT unsigned int GetID()
{
    return LLGL_VERSION_ID;
}

LLGL_EXPORT std::string GetString()
{
    std::stringstream s;

    s << GetMajor() << '.';

    if (GetMinor() < 10)
        s << '0';
    s << GetMinor();

    s << ' ' << GetStatus();

    if (GetRevision())
        s << " (Rev. " << GetRevision() << ')';

    return s.str();
}


} // /namespace Version

} // /namespace LLGL



// ================================================================================
