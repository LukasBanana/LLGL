/*
 * CsShader.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#pragma once

#include <vcclr.h>
#include <LLGL/Shader.h>
#include "CsShaderFlags.h"

#using <System.dll>
#using <System.Core.dll>
#using <System.Runtime.InteropServices.dll>


using namespace System;
using namespace System::Runtime::InteropServices;


namespace SharpLLGL
{


public ref class Shader
{

    public:

        Shader(LLGL::Shader* native);

        property bool HasErrors
        {
            bool get();
        }

        property String^ Report
        {
            String^ get();
        };

        property ShaderType Type
        {
            ShaderType get();
        }

        property SharpLLGL::StageFlags StageFlags
        {
            SharpLLGL::StageFlags get();
        };

    private:

        LLGL::Shader* native_ = nullptr;

    internal:

        property LLGL::Shader* Native
        {
            LLGL::Shader* get();
        }

};


} // /namespace SharpLLGL



// ================================================================================
