/*
 * SamplerArray.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_SAMPLER_ARRAY_H__
#define __LLGL_SAMPLER_ARRAY_H__


#include "Export.h"


namespace LLGL
{


//! Sampler array interface.
class LLGL_EXPORT SamplerArray
{

    public:

        SamplerArray(const SamplerArray&) = delete;
        SamplerArray& operator = (const SamplerArray&) = delete;

        virtual ~SamplerArray()
        {
        }

    protected:

        SamplerArray() = default;

};


} // /namespace LLGL


#endif



// ================================================================================
