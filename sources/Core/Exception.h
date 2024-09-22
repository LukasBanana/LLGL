/*
 * Exception.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_EXCEPTION_H
#define LLGL_EXCEPTION_H


#include <LLGL/Export.h>
#include <LLGL/Container/StringView.h>
#include <string>
#include <cstddef>
#include "MacroUtils.h"


#define LLGL_TRAP(FORMAT, ...) \
    LLGL::Trap(__FUNCTION__, (FORMAT) LLGL_VA_ARGS(__VA_ARGS__))

#define LLGL_TRAP_NOT_IMPLEMENTED(...) \
    LLGL::TrapNotImplemented(__FUNCTION__ LLGL_VA_ARGS(__VA_ARGS__))

#define LLGL_TRAP_FEATURE_NOT_SUPPORTED(FEATURE) \
    LLGL::TrapFeatureNotSupported(__FUNCTION__, FEATURE)

#define LLGL_UNREACHABLE() \
    LLGL_TRAP("reached code path that should be unreachable")


namespace LLGL
{


class Report;

// Primary function to trap execution from an unrecoverable state. This might either throw an exception, abort execution, or break the debugger.
[[noreturn]]
LLGL_EXPORT void Trap(const char* origin, const char* format, ...);

// Traps program execution with the message that the specified assertion that failed.
[[noreturn]]
LLGL_EXPORT void TrapAssertionFailed(const char* origin, const char* expr, const char* details = nullptr, ...);

// Traps program execution with the message that the specified feature is not supported.
[[noreturn]]
LLGL_EXPORT void TrapFeatureNotSupported(const char* origin, const char* featureName);

// Traps program execution with the message that the specified rendering feature is not supported by the renderer (see RenderingFeatures).
[[noreturn]]
LLGL_EXPORT void TrapRenderingFeatureNotSupported(const char* origin, const char* featureName);

// Traps program execution with the message that the specified OpenGL extension is not supported.
[[noreturn]]
LLGL_EXPORT void TrapGLExtensionNotSupported(const char* origin, const char* extensionName, const char* useCase = nullptr);

// Traps program execution with the message that the specified Vulkan extension is not supported.
[[noreturn]]
LLGL_EXPORT void TrapVKExtensionNotSupported(const char* origin, const char* extensionName, const char* useCase = nullptr);

// Traps program execution with the message that the specified interface function is not implemented yet.
[[noreturn]]
LLGL_EXPORT void TrapNotImplemented(const char* origin, const char* useCase = nullptr);

// Traps program execution with the message that a null pointer was passed.
[[noreturn]]
LLGL_EXPORT void TrapNullPointer(const char* origin, const char* expr);

// Traps program execution with the message that a value has exceeded an upper bound, i.e. <value> is not in the half-open range [0, upperBound).
[[noreturn]]
LLGL_EXPORT void TrapParamExceededUpperBound(const char* origin, const char* paramName, int value, int upperBound);

// Traps program execution with the message that a value has exceeded its maximum, i.e. <value> is not in the closed range [0, maximum].
[[noreturn]]
LLGL_EXPORT void TrapParamExceededMaximum(const char* origin, const char* paramName, int value, int maximum);

// Traps program execution with the message from the specified report, cutting off any trailing new-line characters.
[[noreturn]]
LLGL_EXPORT void TrapReport(const char* origin, const Report& report);

// Reports the specified error if 'report' is non-null (see Report::Errorf) or throws a std::runtime_error if LLGL_ENABLE_EXCEPTIONS is defined.
LLGL_EXPORT std::nullptr_t ReportException(Report* report, const char* format, ...);


} // /namespace LLGL


#endif



// ================================================================================
