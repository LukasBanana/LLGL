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


} // /namespace LLGL



// ================================================================================
