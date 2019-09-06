/*
 * CsShaderFlags.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "CsShaderFlags.h"
#include "CsHelper.h"


namespace SharpLLGL
{


VertexShaderAttributes::VertexShaderAttributes()
{
    InputAttribs = gcnew List<VertexAttribute^>();
    OutputAttribs = gcnew List<VertexAttribute^>();
}

ShaderDescriptor::ShaderDescriptor()
{
    Type        = ShaderType::Undefined;
    Source      = "";
    SourceType  = ShaderSourceType::CodeFile;
    EntryPoint  = "";
    Profile     = "";
    Flags       = ShaderCompileFlags::None;
    Vertex      = gcnew VertexShaderAttributes();
    //Fragment    = gcnew FragmentShaderAttributes();
}

ShaderDescriptor::ShaderDescriptor(ShaderType type, String^ source)
{
    Type        = type;
    Source      = source;
    SourceType  = ShaderSourceType::CodeFile;
    EntryPoint  = "";
    Profile     = "";
    Flags       = ShaderCompileFlags::None;
    Vertex      = gcnew VertexShaderAttributes();
    //Fragment    = gcnew FragmentShaderAttributes();
}

ShaderDescriptor::ShaderDescriptor(ShaderType type, String^ source, String^ entryPoint, String^ profile)
{
    Type        = type;
    Source      = source;
    SourceType  = ShaderSourceType::CodeFile;
    EntryPoint  = entryPoint;
    Profile     = profile;
    Flags       = ShaderCompileFlags::None;
    Vertex      = gcnew VertexShaderAttributes();
    //Fragment    = gcnew FragmentShaderAttributes();
}

ShaderDescriptor::ShaderDescriptor(ShaderType type, String^ source, String^ entryPoint, String^ profile, ShaderCompileFlags flags)
{
    Type        = type;
    Source      = source;
    SourceType  = ShaderSourceType::CodeFile;
    EntryPoint  = entryPoint;
    Profile     = profile;
    Flags       = flags;
    Vertex      = gcnew VertexShaderAttributes();
    //Fragment    = gcnew FragmentShaderAttributes();
}

ShaderDescriptor::ShaderDescriptor(ShaderSourceType sourceType, ShaderType type, String^ source)
{
    Type        = type;
    Source      = source;
    SourceType  = sourceType;
    EntryPoint  = "";
    Profile     = "";
    Flags       = ShaderCompileFlags::None;
    Vertex      = gcnew VertexShaderAttributes();
    //Fragment    = gcnew FragmentShaderAttributes();
}

ShaderDescriptor::ShaderDescriptor(ShaderSourceType sourceType, ShaderType type, String^ source, String^ entryPoint, String^ profile)
{
    Type        = type;
    Source      = source;
    SourceType  = sourceType;
    EntryPoint  = entryPoint;
    Profile     = profile;
    Flags       = ShaderCompileFlags::None;
    Vertex      = gcnew VertexShaderAttributes();
    //Fragment    = gcnew FragmentShaderAttributes();
}

ShaderDescriptor::ShaderDescriptor(ShaderSourceType sourceType, ShaderType type, String^ source, String^ entryPoint, String^ profile, ShaderCompileFlags flags)
{
    Type        = type;
    Source      = source;
    SourceType  = sourceType;
    EntryPoint  = entryPoint;
    Profile     = profile;
    Flags       = flags;
    Vertex      = gcnew VertexShaderAttributes();
    //Fragment    = gcnew FragmentShaderAttributes();
}


} // /namespace SharpLLGL



// ================================================================================
