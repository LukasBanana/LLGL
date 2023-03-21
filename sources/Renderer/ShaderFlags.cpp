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

LLGL_EXPORT long GetStageFlags(const ShaderType type)
{
    switch (type)
    {
        case ShaderType::Undefined:         break;
        case ShaderType::Vertex:            return StageFlags::VertexStage;
        case ShaderType::TessControl:       return StageFlags::TessControlStage;
        case ShaderType::TessEvaluation:    return StageFlags::TessEvaluationStage;
        case ShaderType::Geometry:          return StageFlags::GeometryStage;
        case ShaderType::Fragment:          return StageFlags::FragmentStage;
        case ShaderType::Compute:           return StageFlags::ComputeStage;
    }
    return 0;
}


} // /namespace LLGL



// ================================================================================
