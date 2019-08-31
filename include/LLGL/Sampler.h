/*
 * Sampler.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
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
