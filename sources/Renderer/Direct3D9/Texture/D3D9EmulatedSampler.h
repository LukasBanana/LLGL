/*
 * D3D9EmulatedSampler.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_D3D9_EMULATED_SAMPLER_H
#define LLGL_D3D9_EMULATED_SAMPLER_H


#include <LLGL/Sampler.h>
#include "../Direct3D9.h"
#include "../RenderState/D3D9State.h"
#include <memory>


namespace LLGL
{


class D3D9StateManager;

// Similarly to GLEmulatedSampler, the only D3D9 sampler implementation stores pre-translated enum values for sampler states.
class D3D9EmulatedSampler final : public Sampler
{

    public:

        #include <LLGL/Backend/Sampler.inl>

    public:

        void SetDebugName(const char* name) override;

    public:

        D3D9EmulatedSampler(const SamplerDescriptor& desc);

        inline const D3D9SamplerState& GetD3DState() const
        {
            return d3dState_;
        }

    private:

        D3D9SamplerState d3dState_;

};

using D3D9EmulatedSamplerSPtr = std::shared_ptr<D3D9EmulatedSampler>;


} // /namespace LLGL


#endif



// ================================================================================
