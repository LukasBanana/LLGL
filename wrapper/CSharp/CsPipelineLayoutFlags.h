/*
 * CsPipelineLayoutFlags.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#pragma once

#include <vcclr.h>
#include "CsRenderSystemChilds.h"
#include "CsShaderFlags.h"

#using <System.dll>
#using <System.Core.dll>
#using <System.Runtime.InteropServices.dll>


namespace SharpLLGL
{


/* ----- Structures ----- */

public ref class BindingDescriptor
{
    public:

        BindingDescriptor();
        BindingDescriptor(ResourceType type, StageFlags stageFlags, unsigned int slot);
        BindingDescriptor(ResourceType type, StageFlags stageFlags, unsigned int slot, unsigned int arraySize);

        property ResourceType   Type;
        property StageFlags     StageFlags;
        property unsigned int   Slot;
        property unsigned int   ArraySize;

};

public ref class PipelineLayoutDescriptor
{

    public:

        PipelineLayoutDescriptor();

        property List<BindingDescriptor^>^ Bindings;

};


} // /namespace SharpLLGL



// ================================================================================
