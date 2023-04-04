/*
 * Sampler.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_SAMPLER_H
#define LLGL_SAMPLER_H


#include <LLGL/Resource.h>
#include <LLGL/SamplerFlags.h>


namespace LLGL
{


/**
\brief Sampler interface.
\see RenderSystem::CreateSampler
*/
class LLGL_EXPORT Sampler : public Resource
{

        LLGL_DECLARE_INTERFACE( InterfaceID::Sampler );

    public:

        //! Returns ResourceType::Sampler.
        ResourceType GetResourceType() const override final;

    protected:

        Sampler() = default;

};


} // /namespace LLGL


#endif



// ================================================================================
