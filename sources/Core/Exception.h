/*
 * Exception.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_EXCEPTION_H
#define LLGL_EXCEPTION_H


#include <LLGL/Export.h>
#include <string>


namespace LLGL
{


// Throws an std::runtime_error exception with the message, that the specified feature is not supported by the renderer.
[[noreturn]]
LLGL_EXPORT void ThrowNotSupported(const std::string& featureName);

// Throws an std::runtime_error exception with the message, that the specified interface function is not implemented yet.
[[noreturn]]
LLGL_EXPORT void ThrowNotImplemented(const std::string& functionName);


} // /namespace LLGL


#endif



// ================================================================================
