/*
 * C99Log.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include <LLGL/Log.h>
#include <LLGL-C/Log.h>
#include "C99Internal.h"
#include "../../sources/Core/StringUtils.h"


// namespace LLGL {


using namespace LLGL;

LLGL_C_EXPORT void llglLogPrintf(const char* format, ...)
{
    std::string text;
    LLGL_STRING_PRINTF(text, format);
    Log::Printf("%s", text.c_str());
}

LLGL_C_EXPORT void llglLogPrintfExt(const LLGLColorCodes* colors, const char* format, ...)
{
    std::string text;
    LLGL_STRING_PRINTF(text, format);
    Log::Printf(*reinterpret_cast<const Log::ColorCodes*>(colors), "%s", text.c_str());
}

LLGL_C_EXPORT void llglLogErrorf(const char* format, ...)
{
    std::string text;
    LLGL_STRING_PRINTF(text, format);
    Log::Errorf("%s", text.c_str());
}

LLGL_C_EXPORT void llglLogErrorfExt(const LLGLColorCodes* colors, const char* format, ...)
{
    std::string text;
    LLGL_STRING_PRINTF(text, format);
    Log::Errorf(*reinterpret_cast<const Log::ColorCodes*>(colors), "%s", text.c_str());
}

LLGL_C_EXPORT LLGLLogHandle llglRegisterLogCallback(LLGL_PFN_ReportCallback callback, void* userData)
{
    LLGL_ASSERT_PTR(callback);
    return Log::RegisterCallback(
        [callback](Log::ReportType type, const char* text, void* userData) -> void
        {
            callback(static_cast<LLGLReportType>(type), text, userData);
        },
        userData
    );
}

LLGL_C_EXPORT LLGLLogHandle llglRegisterLogCallbackExt(LLGL_PFN_ReportCallbackExt callback, void* userData)
{
    LLGL_ASSERT_PTR(callback);
    return Log::RegisterCallback(
        [callback](Log::ReportType type, const char* text, void* userData, const Log::ColorCodes& colors) -> void
        {
            callback(static_cast<LLGLReportType>(type), text, userData, reinterpret_cast<const LLGLColorCodes*>(&colors));
        },
        userData
    );
}

LLGL_C_EXPORT LLGLLogHandle llglRegisterLogCallbackReport(LLGLReport report)
{
    return Log::RegisterCallbackReport(LLGL_REF(Report, report));
}

LLGL_C_EXPORT LLGLLogHandle llglRegisterLogCallbackStd(long stdOutFlags)
{
    return Log::RegisterCallbackStd(stdOutFlags);
}

LLGL_C_EXPORT void llglUnregisterLogCallback(LLGLLogHandle handle)
{
    Log::UnregisterCallback(handle);
}


// } /namespace LLGL



// ================================================================================
