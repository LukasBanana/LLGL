/*
 * SamplerState.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef __LLGL_SAMPLER_STATE_H__
#define __LLGL_SAMPLER_STATE_H__


#include "Export.h"
#include "SamplerStateFlags.h"


namespace LLGL
{


//! Sampler state interface.
class LLGL_EXPORT SamplerState
{

    public:

        virtual ~SamplerState()
        {
        }

};


} // /namespace LLGL


#endif



// ================================================================================
