/*
 * D3D9Sampler.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "D3D9Sampler.h"


namespace LLGL
{


D3D9Sampler::D3D9Sampler(const SamplerDescriptor& desc) :
    desc { desc }
{
    if (desc.debugName != nullptr)
        SetDebugName(desc.debugName);
}

void D3D9Sampler::SetDebugName(const char* name)
{
    if (name != nullptr)
        label_ = name;
    else
        label_.clear();
}

bool D3D9Sampler::GetNativeHandle(void* /*nativeHandle*/, std::size_t /*nativeHandleSize*/)
{
    return false; // dummy
}


} // /namespace LLGL



// ================================================================================
