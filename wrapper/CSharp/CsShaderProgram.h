/*
 * CsShaderProgram.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#pragma once

#include <vcclr.h>
#include <LLGL/ShaderProgram.h>
#include "CsShaderProgramFlags.h"

#using <System.dll>
#using <System.Core.dll>
#using <System.Runtime.InteropServices.dll>


using namespace System;
using namespace System::Runtime::InteropServices;


namespace SharpLLGL
{


public ref class ShaderProgram
{

    public:

        ShaderProgram(::LLGL::ShaderProgram* native);

        property ::LLGL::ShaderProgram* Native
        {
            ::LLGL::ShaderProgram* get();
        }

        property bool HasErrors
        {
            bool get();
        }

        String^ QueryInfoLog();

        #if 0
        ShaderReflectionDescriptor^ QueryReflectionDesc();
        #endif

        void BindConstantBuffer(String^ name, unsigned int bindingIndex);
        void BindStorageBuffer(String^ name, unsigned int bindingIndex);

        #if 0
        ShaderUniform^ LockShaderUniform();
        void UnlockShaderUniform();
        #endif

    private:

        ::LLGL::ShaderProgram* native_ = nullptr;

};


} // /namespace SharpLLGL



// ================================================================================
