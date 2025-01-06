/*
 * NullSampler.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "NullSampler.h"


namespace LLGL
{


NullSampler::NullSampler(const SamplerDescriptor& desc) :
    desc { desc }
{
    if (desc.debugName != nullptr)
        SetDebugName(desc.debugName);
}

void NullSampler::SetDebugName(const char* name)
{
    if (name != nullptr)
        label_ = name;
    else
        label_.clear();
}

bool NullSampler::GetNativeHandle(void* /*nativeHandle*/, std::size_t /*nativeHandleSize*/)
{
    return false; // dummy
}


} // /namespace LLGL



// ================================================================================
