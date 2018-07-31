/*
 * CsShaderFlags.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "CsShaderFlags.h"
#include "CsHelper.h"


namespace SharpLLGL
{


ShaderDescriptor::ShaderDescriptor()
{
    Type        = ShaderType::Undefined;
    Source      = "";
    SourceType  = ShaderSourceType::CodeFile;
    EntryPoint  = "";
    Profile     = "";
    Flags       = ShaderCompileFlags::None;
}

ShaderDescriptor::ShaderDescriptor(ShaderType type, String^ source)
{
    Type        = type;
    Source      = source;
    SourceType  = ShaderSourceType::CodeFile;
    EntryPoint  = "";
    Profile     = "";
    Flags       = ShaderCompileFlags::None;
}

ShaderDescriptor::ShaderDescriptor(ShaderType type, String^ source, String^ entryPoint, String^ profile)
{
    Type        = type;
    Source      = source;
    SourceType  = ShaderSourceType::CodeFile;
    EntryPoint  = entryPoint;
    Profile     = profile;
    Flags       = ShaderCompileFlags::None;
}

ShaderDescriptor::ShaderDescriptor(ShaderType type, String^ source, String^ entryPoint, String^ profile, ShaderCompileFlags flags)
{
    Type        = type;
    Source      = source;
    SourceType  = ShaderSourceType::CodeFile;
    EntryPoint  = entryPoint;
    Profile     = profile;
    Flags       = flags;
}

ShaderDescriptor::ShaderDescriptor(ShaderSourceType sourceType, ShaderType type, String^ source)
{
    Type        = type;
    Source      = source;
    SourceType  = sourceType;
    EntryPoint  = "";
    Profile     = "";
    Flags       = ShaderCompileFlags::None;
}

ShaderDescriptor::ShaderDescriptor(ShaderSourceType sourceType, ShaderType type, String^ source, String^ entryPoint, String^ profile)
{
    Type        = type;
    Source      = source;
    SourceType  = sourceType;
    EntryPoint  = entryPoint;
    Profile     = profile;
    Flags       = ShaderCompileFlags::None;
}

ShaderDescriptor::ShaderDescriptor(ShaderSourceType sourceType, ShaderType type, String^ source, String^ entryPoint, String^ profile, ShaderCompileFlags flags)
{
    Type        = type;
    Source      = source;
    SourceType  = sourceType;
    EntryPoint  = entryPoint;
    Profile     = profile;
    Flags       = flags;
}


} // /namespace SharpLLGL



// ================================================================================
