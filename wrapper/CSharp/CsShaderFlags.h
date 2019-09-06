/*
 * CsShaderFlags.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#pragma once

#include <vcclr.h>
#include "CsVertexFormat.h"

#using <System.dll>
#using <System.Core.dll>
#using <System.Runtime.InteropServices.dll>


using namespace System;
using namespace System::Runtime::InteropServices;


namespace SharpLLGL
{


/* ---- Enumerations ----- */

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


/* ----- Flags ----- */

[Flags]
public enum class ShaderCompileFlags
{
    None        = 0,
    Debug       = (1 << 0),
    O1          = (1 << 1),
    O2          = (1 << 2),
    O3          = (1 << 3),
    WarnError   = (1 << 4),
};

[Flags]
public enum class StageFlags
{
    None                = 0,
    VertexStage         = (1 << 0),
    TessControlStage    = (1 << 1),
    TessEvaluationStage = (1 << 2),
    GeometryStage       = (1 << 3),
    FragmentStage       = (1 << 4),
    ComputeStage        = (1 << 5),

    AllTessStages       = (TessControlStage | TessEvaluationStage),
    AllGraphicsStages   = (VertexStage | AllTessStages | GeometryStage | FragmentStage),
    AllStages           = (AllGraphicsStages | ComputeStage),
};


/* ----- Structures ----- */

public ref class VertexShaderAttributes
{

    public:

        VertexShaderAttributes();

        property List<VertexAttribute^>^ InputAttribs;
        property List<VertexAttribute^>^ OutputAttribs;

};

/*
public ref class FragmentShaderAttributes
{

    public:

        FragmentShaderAttributes();

        property List<FragmentAttribute^>^ OutputAttribs;

};
*/

public ref class ShaderDescriptor
{

    public:

        ShaderDescriptor();
        ShaderDescriptor(ShaderType type, String^ source);
        ShaderDescriptor(ShaderType type, String^ source, String^ entryPoint, String^ profile);
        ShaderDescriptor(ShaderType type, String^ source, String^ entryPoint, String^ profile, ShaderCompileFlags flags);
        ShaderDescriptor(ShaderSourceType sourceType, ShaderType type, String^ source);
        ShaderDescriptor(ShaderSourceType sourceType, ShaderType type, String^ source, String^ entryPoint, String^ profile);
        ShaderDescriptor(ShaderSourceType sourceType, ShaderType type, String^ source, String^ entryPoint, String^ profile, ShaderCompileFlags flags);

        property ShaderType                 Type;
        property String^                    Source;
        property ShaderSourceType           SourceType;
        property String^                    EntryPoint;
        property String^                    Profile;
        property ShaderCompileFlags         Flags;
        property VertexShaderAttributes^    Vertex;
        //property FragmentShaderAttributes^  Fragment;

};


} // /namespace SharpLLGL



// ================================================================================
