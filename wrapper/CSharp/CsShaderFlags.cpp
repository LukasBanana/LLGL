/*
 * CsShaderFlags.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "CsShaderFlags.h"
#include "CsHelper.h"


namespace LHermanns
{

namespace LLGL
{


ShaderDescriptor::ShaderDescriptor()
{
    Type        = ShaderType::Undefined;
    Source      = "";
    SourceType  = ShaderSourceType::CodeFile;
    EntryPoint  = "";
    Profile     = "";
    Flags       = 0;
}


} // /namespace LLGL

} // /namespace LHermanns



// ================================================================================
