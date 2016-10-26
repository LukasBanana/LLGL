/*
 * Sampler.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_SAMPLER_H
#define LLGL_SAMPLER_H


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
