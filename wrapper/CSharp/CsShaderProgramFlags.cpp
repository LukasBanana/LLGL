/*
 * CsShaderProgramFlags.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "CsShaderProgramFlags.h"
#include "CsHelper.h"


namespace SharpLLGL
{


ShaderProgramDescriptor::ShaderProgramDescriptor()
{
    VertexShader            = nullptr;
    TessControlShader       = nullptr;
    TessEvaluationShader    = nullptr;
    GeometryShader          = nullptr;
    FragmentShader          = nullptr;
    ComputeShader           = nullptr;
}


} // /namespace SharpLLGL



// ================================================================================
