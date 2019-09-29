/*
 * Exception.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "Exception.h"
#include <stdexcept>


namespace LLGL
{


static void AddFuncName(std::string& s, const char* funcName)
{
    s += "in '";
    s += funcName;
    s += "': ";
}

[[noreturn]]
LLGL_EXPORT void ThrowNotSupportedExcept(const char* funcName, const char* featureName)
{
    std::string s;
    {
        AddFuncName(s, funcName);
        s += featureName;
        s += " not supported";
    }
    throw std::runtime_error(s);
}

[[noreturn]]
LLGL_EXPORT void ThrowRenderingFeatureNotSupportedExcept(const char* funcName, const char* featureName)
{
    std::string s;
    {
        AddFuncName(s, funcName);
        s += "LLGL::RenderingFeatures::";
        s += featureName;
        s += " not supported";
    }
    throw std::runtime_error(s);
}

[[noreturn]]
LLGL_EXPORT void ThrowGLExtensionNotSupportedExcept(const char* funcName, const char* extensionName)
{
    std::string s;
    {
        AddFuncName(s, funcName);
        s += "OpenGL extension '";
        s += extensionName;
        s += "' not supported";
    }
    throw std::runtime_error(s);
}

[[noreturn]]
LLGL_EXPORT void ThrowVKExtensionNotSupportedExcept(const char* funcName, const char* extensionName)
{
    std::string s;
    {
        AddFuncName(s, funcName);
        s += "Vulkan extension '";
        s += extensionName;
        s += "' not supported";
    }
    throw std::runtime_error(s);
}

[[noreturn]]
LLGL_EXPORT void ThrowNotImplementedExcept(const char* funcName)
{
    std::string s;
    {
        AddFuncName(s, funcName);
        s += "not implemented yet";
    }
    throw std::runtime_error(std::string(funcName) + ": not implemented yet");
}

[[noreturn]]
LLGL_EXPORT void ThrowNullPointerExcept(const char* funcName, const char* paramName)
{
    std::string s;
    {
        AddFuncName(s, funcName);
        s += "parameter '";
        s += paramName;
        s += "' must not be a null pointer";
    }
    throw std::invalid_argument(s);
}

[[noreturn]]
LLGL_EXPORT void ThrowExceededUpperBoundExcept(const char* funcName, const char* paramName, int value, int upperBound)
{
    std::string s;
    {
        AddFuncName(s, funcName);
        s += "parameter '";
        s += paramName;
        s += " = ";
        s += std::to_string(value);
        s += "' out of half-open range [0, ";
        s += std::to_string(upperBound);
        s += ")";
    }
    throw std::out_of_range(s);
}

[[noreturn]]
LLGL_EXPORT void ThrowExceededMaximumExcept(const char* funcName, const char* paramName, int value, int maximum)
{
    std::string s = funcName;
    {
        AddFuncName(s, funcName);
        s += "parameter '";
        s += paramName;
        s += " = ";
        s += std::to_string(value);
        s += "' out of range [0, ";
        s += std::to_string(maximum);
        s += "]";
    }
    throw std::out_of_range(s);
}


} // /namespace LLGL



// ================================================================================
