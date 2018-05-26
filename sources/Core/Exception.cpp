/*
 * Exception.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "Exception.h"
#include <stdexcept>


namespace LLGL
{


[[noreturn]]
LLGL_EXPORT void ThrowNotSupportedExcept(const char* funcName, const char* featureName)
{
    throw std::runtime_error(std::string(funcName) + ": " + std::string(featureName) + " not supported");
}

[[noreturn]]
LLGL_EXPORT void ThrowRenderingFeatureNotSupportedExcept(const char* funcName, const char* featureName)
{
    std::string s = funcName;
    {
        s += ": LLGL::RenderingFeatures::";
        s += featureName;
        s += " not supported";
    }
    throw std::runtime_error(s);
}

[[noreturn]]
LLGL_EXPORT void ThrowNotImplementedExcept(const char* funcName)
{
    throw std::runtime_error(std::string(funcName) + ": not implemented yet");
}

[[noreturn]]
LLGL_EXPORT void ThrowNullPointerExcept(const char* funcName, const char* paramName)
{
    throw std::runtime_error(std::string(funcName) + ": null pointer passed to parameter: " + std::string(paramName));
}

[[noreturn]]
LLGL_EXPORT void ThrowExceededUpperBoundExcept(const char* funcName, const char* paramName, int value, int upperBound)
{
    std::string s = funcName;
    {
        s += ": parameter out of half-open range (";
        s += std::to_string(value);
        s += " is specified, but upper bound is ";
        s += std::to_string(upperBound);
        s += "): ";
        s += paramName;
    }
    throw std::out_of_range(s);
}

[[noreturn]]
LLGL_EXPORT void ThrowExceededMaximumExcept(const char* funcName, const char* paramName, int value, int maximum)
{
    std::string s = funcName;
    {
        s += ": parameter out of range (";
        s += std::to_string(value);
        s += " is specified, but maximum is ";
        s += std::to_string(maximum);
        s += "): ";
        s += paramName;
    }
    throw std::out_of_range(s);
}


} // /namespace LLGL



// ================================================================================
