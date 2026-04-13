/*
 * WGSampler.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_WG_SAMPLER_H
#define LLGL_WG_SAMPLER_H


#include <LLGL/Sampler.h>
#include <webgpu/webgpu.h>


namespace LLGL
{


class WGSampler final : public Sampler
{

    public:

        #include <LLGL/Backend/Sampler.inl>

    public:

        WGSampler(WGPUDevice device, const SamplerDescriptor& desc);
        ~WGSampler();

        // Returns the native WebGPU sampler handle.
        inline WGPUSampler GetNative() const
        {
            return sampler_;
        }

    private:

        WGPUSampler sampler_ = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
