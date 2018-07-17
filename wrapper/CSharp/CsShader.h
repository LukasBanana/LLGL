/*
 * CsShader.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
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


namespace LHermanns
{

namespace LLGL
{


public ref class Shader
{

    public:

        Shader(::LLGL::Shader* native);

        property ::LLGL::Shader* Native
        {
            ::LLGL::Shader* get();
        }

        property bool HasErrors
        {
            bool get();
        }

        String^ Disassemble();
        String^ Disassemble(int flags);

        String^ QueryInfoLog();

        property ShaderType Type
        {
            ShaderType get();
        }

        property int StageFlags
        {
            int get();
        };

    private:

        ::LLGL::Shader* native_ = nullptr;

};


} // /namespace LLGL

} // /namespace LHermanns



// ================================================================================
