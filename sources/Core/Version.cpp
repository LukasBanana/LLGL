/*
 * Version.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/Version.h>
#include "VersionMacros.h"
#include "MacroUtils.h"
#include <stdio.h>
#include <string>


namespace LLGL
{


int Compare(VersionInfo lhs, VersionInfo rhs)
{
    LLGL_COMPARE_MEMBER_SWO(major);
    LLGL_COMPARE_MEMBER_SWO(minor);
    LLGL_COMPARE_MEMBER_SWO(revision);
    LLGL_COMPARE_MEMBER_SWO(status);
    return 0;
}

LLGL_EXPORT const char* ToString(VersionStatus status)
{
    switch (status)
    {
        case VersionStatus::Undefined:  break;
        case VersionStatus::Alpha:      return "Alpha";
        case VersionStatus::Beta:       return "Beta";
        case VersionStatus::Stable:     return "Stable";
    }
    return nullptr;
}

const char* ToString(VersionInfo info)
{
    static thread_local char versionInfoString[64];
    return ToString(info, sizeof(versionInfoString), versionInfoString);
}

const char* ToString(VersionInfo info, std::size_t bufferSize, char* buffer, std::size_t* outMinBufferSize)
{
    /* Format the version string and write it to the output buffer */
    int written = 0;
    if (info.status != VersionStatus::Undefined && info.revision > 0)
    {
        written = ::snprintf(
            buffer, bufferSize, "%d.%02d %s (Rev. %d)",
            static_cast<int>(info.major), static_cast<int>(info.minor), ToString(info.status), static_cast<int>(info.revision)
        );
    }
    else if (info.status != VersionStatus::Undefined)
    {
        written = ::snprintf(
            buffer, bufferSize, "%d.%02d %s",
            static_cast<int>(info.major), static_cast<int>(info.minor), ToString(info.status)
        );
    }
    else if (info.revision > 0)
    {
        written = ::snprintf(
            buffer, bufferSize, "%d.%02d (Rev. %d)",
            static_cast<int>(info.major), static_cast<int>(info.minor), static_cast<int>(info.revision)
        );
    }
    else
    {
        written = ::snprintf(
            buffer, bufferSize, "%d.%02d",
            static_cast<int>(info.major), static_cast<int>(info.minor)
        );
    }

    /* Write out the required buffer size */
    if (outMinBufferSize != nullptr && written >= 0)
        *outMinBufferSize = static_cast<std::size_t>(written);

    return buffer;
}

VersionInfo GetLLGLVersion()
{
    VersionInfo info;
    {
        info.major      = LLGL_VERSION_MAJOR;
        info.minor      = LLGL_VERSION_MINOR;
        info.revision   = LLGL_VERSION_REVISION;
        info.status     = VersionStatus::Beta;
    }
    return info;
}


} // /namespace LLGL



// ================================================================================
