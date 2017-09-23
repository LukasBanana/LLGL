/*
 * VKSamplerArray.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKSamplerArray.h"
#include "VKSampler.h"
#include "../../../Core/Helper.h"


namespace LLGL
{


VKSamplerArray::VKSamplerArray(uint32_t numSamplers, Sampler* const * samplerArray)
{
    /* Store the ID of each GLSampler inside the array */
    samplers_.reserve(numSamplers);
    while (auto next = NextArrayResource<VKSampler>(numSamplers, samplerArray))
        samplers_.push_back(next->Get());
}


} // /namespace LLGL



// ================================================================================
