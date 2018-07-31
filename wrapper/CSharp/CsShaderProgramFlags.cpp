/*
 * CsShaderProgramFlags.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "CsShaderProgramFlags.h"
#include "CsHelper.h"


namespace LHermanns
{

namespace SharpLLGL
{


ShaderProgramDescriptor::ShaderProgramDescriptor()
{
    VertexFormats           = gcnew List<VertexFormat^>();
    VertexShader            = nullptr;
    TessControlShader       = nullptr;
    TessEvaluationShader    = nullptr;
    GeometryShader          = nullptr;
    FragmentShader          = nullptr;
    ComputeShader           = nullptr;
}


} // /namespace SharpLLGL

} // /namespace LHermanns



// ================================================================================
