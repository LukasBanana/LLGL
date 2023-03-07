/*
 * Shader.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include <LLGL/Shader.h>


namespace LLGL
{


Shader::Shader(const ShaderType type) :
    type_ { type }
{
}

long Shader::GetStageFlags() const
{
    switch (GetType())
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

bool Shader::IsPostTessellationVertex() const
{
    return false; // dummy
}


} // /namespace LLGL



// ================================================================================
