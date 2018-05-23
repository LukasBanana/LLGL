/*
 * Sampler.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_SAMPLER_H
#define LLGL_SAMPLER_H


#include "Resource.h"
#include "SamplerFlags.h"


namespace LLGL
{


/**
\brief Sampler interface.
\see RenderSystem::CreateSampler
*/
class LLGL_EXPORT Sampler : public Resource
{

    public:

        //! Returns ResourceType::Sampler.
        ResourceType QueryResourceType() const override;

    protected:

        Sampler() = default;

};


} // /namespace LLGL


#endif



// ================================================================================
