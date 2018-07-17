/*
 * CsShaderFlags.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#pragma once

#include <vcclr.h>

#using <System.dll>
#using <System.Core.dll>
#using <System.Runtime.InteropServices.dll>


using namespace System;
using namespace System::Runtime::InteropServices;


namespace LHermanns
{

namespace LLGL
{


public enum class ShaderType
{
    Undefined,
    Vertex,
    TessControl,
    TessEvaluation,
    Geometry,
    Fragment,
    Compute,
};

public enum class ShaderSourceType
{
    CodeString,
    CodeFile,
    BinaryBuffer,
    BinaryFile,
};

public ref class ShaderDescriptor
{

    public:

        #if 0
        ref class StreamOutput
        {

            public:

                StreamOutput();

                property StreamOutputFormat^ Format;

        };
        #endif

    public:

        ShaderDescriptor();
        ShaderDescriptor(ShaderType type, String^ source);
        ShaderDescriptor(ShaderType type, String^ source, String^ entryPoint, String^ profile);
        ShaderDescriptor(ShaderType type, String^ source, String^ entryPoint, String^ profile, int flags);
        ShaderDescriptor(ShaderSourceType sourceType, ShaderType type, String^ source);
        ShaderDescriptor(ShaderSourceType sourceType, ShaderType type, String^ source, String^ entryPoint, String^ profile);
        ShaderDescriptor(ShaderSourceType sourceType, ShaderType type, String^ source, String^ entryPoint, String^ profile, int flags);

        property ShaderType         Type;
        property String^            Source;
        property ShaderSourceType   SourceType;
        property String^            EntryPoint;
        property String^            Profile;
        property int                Flags;
        #if 0
        property StreamOutput^      StreamOutput;
        #endif

};


} // /namespace LLGL

} // /namespace LHermanns



// ================================================================================
