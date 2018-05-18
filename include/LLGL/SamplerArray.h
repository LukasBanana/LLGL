/*
 * SamplerArray.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_SAMPLER_ARRAY_H
#define LLGL_SAMPLER_ARRAY_H


#include "Export.h"


namespace LLGL
{


/**
\breif Sampler container interface.
\todo Maybe rename this to "SamplerHeap".
*/
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
