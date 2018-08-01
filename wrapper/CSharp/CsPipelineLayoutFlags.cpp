/*
 * CsPipelineLayoutFlags.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
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
    StageFlags  = SharpLLGL::StageFlags::None;
    Slot        = 0;
    ArraySize   = 1;
}

BindingDescriptor::BindingDescriptor(ResourceType type, SharpLLGL::StageFlags stageFlags, unsigned int slot)
{
    Type        = type;
    StageFlags  = stageFlags;
    Slot        = slot;
    ArraySize   = 1;
}

BindingDescriptor::BindingDescriptor(ResourceType type, SharpLLGL::StageFlags stageFlags, unsigned int slot, unsigned int arraySize)
{
    Type        = type;
    StageFlags  = stageFlags;
    Slot        = slot;
    ArraySize   = arraySize;
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
