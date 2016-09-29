/*
 * Sampler.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_SAMPLER_H__
#define __LLGL_SAMPLER_H__


#include "Export.h"
#include "SamplerFlags.h"


namespace LLGL
{


//! Sampler interface.
class LLGL_EXPORT Sampler
{

    public:

        Sampler(const Sampler&) = delete;
        Sampler& operator = (const Sampler&) = delete;

        virtual ~Sampler()
        {
        }

    protected:

        Sampler() = default;

};


} // /namespace LLGL


#endif



// ================================================================================
