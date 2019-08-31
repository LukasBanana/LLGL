/*
 * Exception.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_EXCEPTION_H
#define LLGL_EXCEPTION_H


#include <LLGL/Export.h>
#include <string>


namespace LLGL
{


// Throws an std::runtime_error exception with the message, that the specified feature is not supported.
[[noreturn]]
LLGL_EXPORT void ThrowNotSupportedExcept(const char* funcName, const char* featureName);

// Throws an std::runtime_error exception with the message, that the specified rendering feature is not supported by the renderer (see RenderingFeatures).
[[noreturn]]
LLGL_EXPORT void ThrowRenderingFeatureNotSupportedExcept(const char* funcName, const char* featureName);

// Throws an std::runtime_error exception with the message, that the specified OpenGL extension is not supported.
[[noreturn]]
LLGL_EXPORT void ThrowGLExtensionNotSupportedExcept(const char* funcName, const char* extensionName);

// Throws an std::runtime_error exception with the message, that the specified Vulkan extension is not supported.
[[noreturn]]
LLGL_EXPORT void ThrowVKExtensionNotSupportedExcept(const char* funcName, const char* extensionName);

// Throws an std::runtime_error exception with the message, that the specified interface function is not implemented yet.
[[noreturn]]
LLGL_EXPORT void ThrowNotImplementedExcept(const char* funcName);

// Throws an std::invalid_argument exception with the message, that a null pointer was passed.
[[noreturn]]
LLGL_EXPORT void ThrowNullPointerExcept(const char* funcName, const char* paramName);

// Throws an std::out_of_range exception with the message, that a value has exceeded an upper bound, i.e. <value> is not in the half-open range [0, upperBound).
[[noreturn]]
LLGL_EXPORT void ThrowExceededUpperBoundExcept(const char* funcName, const char* paramName, int value, int upperBound);

// Throws an std::out_of_range exception with the message, that a value has exceeded its maximum, i.e. <value> is not in the closed range [0, maximum].
[[noreturn]]
LLGL_EXPORT void ThrowExceededMaximumExcept(const char* funcName, const char* paramName, int value, int maximum);


} // /namespace LLGL


#endif



// ================================================================================
