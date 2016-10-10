/*
 * Exception.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "Exception.h"
#include <stdexcept>


namespace LLGL
{


[[noreturn]]
LLGL_EXPORT void ThrowNotSupported(const std::string& featureName)
{
    throw std::runtime_error("renderer does not supported " + featureName);
}

[[noreturn]]
LLGL_EXPORT void ThrowNotImplemented(const std::string& functionName)
{
    throw std::runtime_error(functionName + " function is not implemented yet");
}


} // /namespace LLGL



// ================================================================================
