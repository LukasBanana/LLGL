/*
 * NullSampler.cpp
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
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
