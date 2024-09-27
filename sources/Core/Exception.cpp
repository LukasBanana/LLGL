/*
 * Exception.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "Exception.h"
#include "CoreUtils.h"
#include "StringUtils.h"
#include <LLGL/Report.h>
#include "../Platform/Debug.h"
#include <stdexcept>
#include <stdarg.h>
#include <stdlib.h>

#include <LLGL/Platform/Platform.h>
#ifdef LLGL_OS_ANDROID
#   include <android/log.h>
#endif


namespace LLGL
{


static void AddOptionalOrigin(std::string& s, const char* origin)
{
    if (origin != nullptr && *origin != '\0')
    {
        s += "in '";
        s += origin;
        s += "': ";
    }
}

[[noreturn]]
LLGL_EXPORT void Trap(const char* origin, const char* format, ...)
{
    /* Build full report string */
    std::string report;
    AddOptionalOrigin(report, origin);

    LLGL_STRING_PRINTF(report, format);

    #if LLGL_ENABLE_EXCEPTIONS

    /* Throw exception with report and optional origin */
    throw std::runtime_error(report);

    #else // LLGL_ENABLE_EXCEPTIONS

    #   ifdef LLGL_DEBUG

    /* Print debug report */
    report = DebugStackTrace().c_str() + report;

    DebugPuts(report.c_str());

    /* Break execution if there's a debugger attached */
    LLGL_DEBUG_BREAK();

    #   else // LLGL_DEBUG

    #       ifdef LLGL_OS_ANDROID

    /* Print report to Android specific error log */
    (void)__android_log_print(ANDROID_LOG_ERROR, "LLGL", "%s\n", report.c_str());

    #       else

    /* Print report to standard error output */
    ::fprintf(stderr, "%s\n", report.c_str());

    #       endif

    #   endif // /LLGL_DEBUG

    /* Abort execution as LLGL is trapped in an unrecoverable state */
    ::abort();

    #endif // /LLGL_ENABLE_EXCEPTIONS
}

[[noreturn]]
LLGL_EXPORT void TrapAssertionFailed(const char* origin, const char* expr, const char* details, ...)
{
    if (details != nullptr && *details != '\0')
    {
        std::string detailsStr;
        LLGL_STRING_PRINTF(detailsStr, details);

        if (!detailsStr.empty())
            Trap(origin, "assertion failed: '%s'; %s", expr, detailsStr.c_str());
        else
            Trap(origin, "assertion failed: '%s'", expr);
    }
    else
        Trap(origin, "assertion failed: '%s'", expr);
}

[[noreturn]]
LLGL_EXPORT void TrapFeatureNotSupported(const char* origin, const char* featureName)
{
    Trap(origin, "%s not supported", featureName);
}

[[noreturn]]
LLGL_EXPORT void TrapRenderingFeatureNotSupported(const char* origin, const char* featureName)
{
    Trap(origin, "LLGL::RenderingFeatures::%s not supported", featureName);
}

[[noreturn]]
LLGL_EXPORT void TrapGLExtensionNotSupported(const char* origin, const char* extensionName, const char* useCase)
{
    if (useCase != nullptr && *useCase != '\0')
        Trap(origin, "OpenGL extension '%s' not supported; required for %s", extensionName, useCase);
    else
        Trap(origin, "OpenGL extension '%s' not supported", extensionName);
}

[[noreturn]]
LLGL_EXPORT void TrapVKExtensionNotSupported(const char* origin, const char* extensionName, const char* useCase)
{
    if (useCase != nullptr && *useCase != '\0')
        Trap(origin, "Vulkan extension '%s' not supported; required for %s", extensionName, useCase);
    else
        Trap(origin, "Vulkan extension '%s' not supported", extensionName);
}

[[noreturn]]
LLGL_EXPORT void TrapNotImplemented(const char* origin, const char* useCase)
{
    if (useCase != nullptr && *useCase != '\0')
        Trap(origin, "not implemented yet: %s", useCase);
    else
        Trap(origin, "not implemented yet");
}

[[noreturn]]
LLGL_EXPORT void TrapNullPointer(const char* origin, const char* expr)
{
    Trap(origin, "expression '%s' must not be null", expr);
}

[[noreturn]]
LLGL_EXPORT void TrapParamExceededUpperBound(const char* origin, const char* paramName, int value, int upperBound)
{
    Trap(origin, "parameter '%s = %d' out of half-open range [0, %d)", paramName, value, upperBound);
}

[[noreturn]]
LLGL_EXPORT void TrapParamExceededMaximum(const char* origin, const char* paramName, int value, int maximum)
{
    Trap(origin, "parameter '%s = %d' out of range [0, %d]", paramName, value, maximum);
}

[[noreturn]]
LLGL_EXPORT void TrapReport(const char* origin, const Report& report)
{
    std::string text = report.GetText();
    for (std::size_t n = text.size(); n > 0 && (text[n - 1] == '\n' || text[n - 1] == '\r'); --n)
        text.pop_back();
    Trap(origin, "%s", text.c_str());
}

LLGL_EXPORT std::nullptr_t ReportException(Report* report, const char* format, ...)
{
    #if LLGL_ENABLE_EXCEPTIONS

    std::string errorStr;
    LLGL_STRING_PRINTF(errorStr, format);
    throw std::runtime_error(errorStr);

    #else // LLGL_ENABLE_EXCEPTIONS

    if (report != nullptr)
    {
        std::string errorStr;
        LLGL_STRING_PRINTF(errorStr, format);
        report->Errorf("%s\n", errorStr.c_str());
    }

    return nullptr;

    #endif // /LLGL_ENABLE_EXCEPTIONS
}


} // /namespace LLGL



// ================================================================================
