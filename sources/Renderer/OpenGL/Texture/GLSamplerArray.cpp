/*
 * GLSamplerArray.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "GLSamplerArray.h"
#include "GLSampler.h"
#include "../../../Core/Helper.h"


namespace LLGL
{


GLSamplerArray::GLSamplerArray(unsigned int numSamplers, Sampler* const * samplerArray)
{
    /* Store the ID of each GLSampler inside the array */
    idArray_.reserve(numSamplers);
    while (auto next = NextArrayResource<GLSampler>(numSamplers, samplerArray))
        idArray_.push_back(next->GetID());
}


} // /namespace LLGL



// ================================================================================
