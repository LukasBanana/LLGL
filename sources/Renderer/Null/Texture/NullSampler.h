/*
 * NullSampler.h
 *
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
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

        void SetName(const char* name) override;

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
