/*
 * NullSampler.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_NULL_SAMPLER_H
#define LLGL_NULL_SAMPLER_H


#include <LLGL/Sampler.h>
#include <string>


namespace LLGL
{


class NullSampler final : public Sampler
{

    public:

        #include <LLGL/Backend/Sampler.inl>

    public:

        void SetDebugName(const char* name) override;

    public:

        NullSampler(const SamplerDescriptor& desc);

    public:

        const SamplerDescriptor desc;

    private:

        std::string label_;

};


} // /namespace LLGL


#endif



// ================================================================================
