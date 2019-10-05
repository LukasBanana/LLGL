/*
 * CsPipelineLayoutFlags.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#pragma once

#include <vcclr.h>
#include "CsRenderSystemChild.h"
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
        BindingDescriptor(ResourceType type, BindFlags bindFlags, StageFlags stageFlags, unsigned int slot);
        BindingDescriptor(ResourceType type, BindFlags bindFlags, StageFlags stageFlags, unsigned int slot, unsigned int arraySize);
        BindingDescriptor(ResourceType type, BindFlags bindFlags, StageFlags stageFlags, unsigned int slot, unsigned int arraySize, String^ name);

        property ResourceType   Type;
        property BindFlags      BindFlags;
        property StageFlags     StageFlags;
        property unsigned int   Slot;
        property unsigned int   ArraySize;
        property String^        Name;

};

public ref class PipelineLayoutDescriptor
{

    public:

        PipelineLayoutDescriptor();

        property List<BindingDescriptor^>^ Bindings;

};


} // /namespace SharpLLGL



// ================================================================================
