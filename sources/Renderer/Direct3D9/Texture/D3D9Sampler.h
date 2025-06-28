/*
 * D3D9Sampler.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D9_SAMPLER_H
#define LLGL_D3D9_SAMPLER_H


#include <LLGL/Sampler.h>
#include <string>


namespace LLGL
{


class D3D9Sampler final : public Sampler
{

    public:

        #include <LLGL/Backend/Sampler.inl>

    public:

        void SetDebugName(const char* name) override;

    public:

        D3D9Sampler(const SamplerDescriptor& desc);

    public:

        const SamplerDescriptor desc;

    private:

        std::string label_;

};


} // /namespace LLGL


#endif



// ================================================================================
