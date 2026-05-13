/*
 * OpenXRError.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_OPENXR_ERROR_H
#define LLGL_OPENXR_ERROR_H


#include <LLGL/Report.h>
#include "../../Core/StringUtils.h"

#include "OpenXRPlatform.h"

#include <cstdarg>
#include <cstdio>


namespace LLGL
{

namespace OpenXR
{


// Returns true if the given XrResult is a failure code (negative in OpenXR's convention).
inline bool Failed(XrResult result) { return result < 0; }

// Returns a static name for a small set of OpenXR result codes that callers commonly
// hit before an XrInstance exists (so xrResultToString isn't usable). Returns nullptr
// if the code is not in the table.
inline const char* StaticResultName(XrResult result)
{
    switch (result)
    {
        case XR_SUCCESS:                                return "XR_SUCCESS";
        case XR_ERROR_VALIDATION_FAILURE:               return "XR_ERROR_VALIDATION_FAILURE";
        case XR_ERROR_RUNTIME_FAILURE:                  return "XR_ERROR_RUNTIME_FAILURE";
        case XR_ERROR_OUT_OF_MEMORY:                    return "XR_ERROR_OUT_OF_MEMORY";
        case XR_ERROR_API_VERSION_UNSUPPORTED:          return "XR_ERROR_API_VERSION_UNSUPPORTED";
        case XR_ERROR_INITIALIZATION_FAILED:            return "XR_ERROR_INITIALIZATION_FAILED";
        case XR_ERROR_FUNCTION_UNSUPPORTED:             return "XR_ERROR_FUNCTION_UNSUPPORTED";
        case XR_ERROR_FEATURE_UNSUPPORTED:              return "XR_ERROR_FEATURE_UNSUPPORTED";
        case XR_ERROR_EXTENSION_NOT_PRESENT:            return "XR_ERROR_EXTENSION_NOT_PRESENT";
        case XR_ERROR_LIMIT_REACHED:                    return "XR_ERROR_LIMIT_REACHED";
        case XR_ERROR_SIZE_INSUFFICIENT:                return "XR_ERROR_SIZE_INSUFFICIENT";
        case XR_ERROR_HANDLE_INVALID:                   return "XR_ERROR_HANDLE_INVALID";
        case XR_ERROR_INSTANCE_LOST:                    return "XR_ERROR_INSTANCE_LOST";
        case XR_ERROR_RUNTIME_UNAVAILABLE:              return "XR_ERROR_RUNTIME_UNAVAILABLE";
        case XR_ERROR_NAME_INVALID:                     return "XR_ERROR_NAME_INVALID";
        case XR_ERROR_FORM_FACTOR_UNSUPPORTED:          return "XR_ERROR_FORM_FACTOR_UNSUPPORTED";
        case XR_ERROR_FORM_FACTOR_UNAVAILABLE:          return "XR_ERROR_FORM_FACTOR_UNAVAILABLE";
        case XR_ERROR_VIEW_CONFIGURATION_TYPE_UNSUPPORTED: return "XR_ERROR_VIEW_CONFIGURATION_TYPE_UNSUPPORTED";
        default:                                        return nullptr;
    }
}

// Returns the OpenXR-runtime-reported string for the given XrResult.
// Tries (in order): runtime lookup via xrResultToString (only possible with a valid XrInstance),
// a static fallback table for common pre-instance errors, then the numeric "XR_UNKNOWN(n)" form.
//
// Defined inline so both LLGL_XR_OpenXR and renderer-side XR binding targets can use it
// without a cross-library link dependency.
inline const char* ResultToString(XrInstance instance, XrResult result)
{
    static thread_local char buffer[XR_MAX_RESULT_STRING_SIZE];
    if (instance != XR_NULL_HANDLE && XR_SUCCEEDED(xrResultToString(instance, result, buffer)))
        return buffer;
    if (const char* staticName = StaticResultName(result))
        return staticName;
    std::snprintf(buffer, sizeof(buffer), "XR_UNKNOWN(%d)", static_cast<int>(result));
    return buffer;
}

// Reports an OpenXR failure into the given Report object.
inline void ReportXrError(Report* report, XrInstance instance, XrResult result, const char* contextLabel)
{
    if (report == nullptr)
        return;
    report->Errorf("%s failed: %s\n", contextLabel, ResultToString(instance, result));
}

// Reports an OpenXR failure into the given Report object using a printf-style label.
inline void ReportXrErrorf(Report* report, XrInstance instance, XrResult result, const char* format, ...)
{
    if (report == nullptr)
        return;

    std::string detailsStr;
    LLGL_STRING_PRINTF(detailsStr, format);

    report->Errorf("%s failed: %s\n", detailsStr.c_str(), ResultToString(instance, result));
}


} // /namespace OpenXR

} // /namespace LLGL


#endif



// ================================================================================
