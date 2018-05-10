/*
 * Strings.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/Strings.h>


namespace LLGL
{


LLGL_EXPORT const char* ToString(const ShaderType t)
{
    switch (t)
    {
        case ShaderType::Vertex:            return "vertex";
        case ShaderType::TessControl:       return "tessellation control";
        case ShaderType::TessEvaluation:    return "tessellation evaluation";
        case ShaderType::Geometry:          return "geometry";
        case ShaderType::Fragment:          return "fragment";
        case ShaderType::Compute:           return "compute";
    }
    return nullptr;
}

LLGL_EXPORT const char* ToString(const ErrorType t)
{
    switch (t)
    {
        case ErrorType::InvalidArgument:    return "invalid argument";
        case ErrorType::InvalidState:       return "invalid state";
        case ErrorType::UnsupportedFeature: return "unsupported feature";
        case ErrorType::UndefinedBehavior:  return "undefined behavior";
    }
    return nullptr;
}

LLGL_EXPORT const char* ToString(const WarningType t)
{
    switch (t)
    {
        case WarningType::ImproperArgument:     return "improper argument";
        case WarningType::ImproperState:        return "improper state";
        case WarningType::PointlessOperation:   return "pointless operation";
    }
    return nullptr;
}


} // /namespace LLGL



// ================================================================================
