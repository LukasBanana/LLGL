/*
 * ShaderFlags.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/ShaderFlags.h>


namespace LLGL
{


LLGL_EXPORT bool IsShaderSourceCode(const ShaderSourceType type)
{
    return (type == ShaderSourceType::CodeString || type == ShaderSourceType::CodeFile);
}

LLGL_EXPORT bool IsShaderSourceBinary(const ShaderSourceType type)
{
    return (type == ShaderSourceType::BinaryBuffer || type == ShaderSourceType::BinaryFile);
}


} // /namespace LLGL



// ================================================================================
