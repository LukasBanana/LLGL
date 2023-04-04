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
}

void NullSampler::SetName(const char* name)
{
    if (name != nullptr)
        label_ = name;
    else
        label_.clear();
}


} // /namespace LLGL



// ================================================================================
