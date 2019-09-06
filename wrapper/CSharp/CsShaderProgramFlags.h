/*
 * CsShaderProgramFlags.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#pragma once

#include <vcclr.h>
#include "CsVertexFormat.h"
#include "CsShader.h"

#using <System.dll>
#using <System.Core.dll>
#using <System.Runtime.InteropServices.dll>


using namespace System;
using namespace System::Runtime::InteropServices;
using namespace System::Collections::Generic;


namespace SharpLLGL
{


public ref class ShaderProgramDescriptor
{

    public:

        ShaderProgramDescriptor();

        property Shader^ VertexShader;
        property Shader^ TessControlShader;
        property Shader^ TessEvaluationShader;
        property Shader^ GeometryShader;
        property Shader^ FragmentShader;
        property Shader^ ComputeShader;

};


} // /namespace SharpLLGL



// ================================================================================
