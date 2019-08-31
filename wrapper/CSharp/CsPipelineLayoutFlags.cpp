/*
 * CsPipelineLayoutFlags.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "CsPipelineLayoutFlags.h"


namespace SharpLLGL
{


/*
 * BindingDescriptor class
 */

BindingDescriptor::BindingDescriptor()
{
    Type        = ResourceType::Undefined;
    BindFlags   = SharpLLGL::BindFlags::None;
    StageFlags  = SharpLLGL::StageFlags::None;
    Slot        = 0;
    ArraySize   = 1;
    Name        = gcnew String("");
}

BindingDescriptor::BindingDescriptor(ResourceType type, SharpLLGL::BindFlags bindFlags, SharpLLGL::StageFlags stageFlags, unsigned int slot)
{
    Type        = type;
    BindFlags   = bindFlags;
    StageFlags  = stageFlags;
    Slot        = slot;
    ArraySize   = 1;
    Name        = gcnew String("");
}

BindingDescriptor::BindingDescriptor(ResourceType type, SharpLLGL::BindFlags bindFlags, SharpLLGL::StageFlags stageFlags, unsigned int slot, unsigned int arraySize)
{
    Type        = type;
    BindFlags   = bindFlags;
    StageFlags  = stageFlags;
    Slot        = slot;
    ArraySize   = arraySize;
    Name        = gcnew String("");
}

BindingDescriptor::BindingDescriptor(ResourceType type, SharpLLGL::BindFlags bindFlags, SharpLLGL::StageFlags stageFlags, unsigned int slot, unsigned int arraySize, String^ name)
{
    Type        = type;
    BindFlags   = bindFlags;
    StageFlags  = stageFlags;
    Slot        = slot;
    ArraySize   = arraySize;
    Name        = name;
}


/*
 * PipelineLayoutDescriptor class
 */

PipelineLayoutDescriptor::PipelineLayoutDescriptor()
{
    Bindings = gcnew List<BindingDescriptor^>();
}


} // /namespace SharpLLGL



// ================================================================================
