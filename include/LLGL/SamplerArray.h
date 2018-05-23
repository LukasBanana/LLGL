/*
 * SamplerArray.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_SAMPLER_ARRAY_H
#define LLGL_SAMPLER_ARRAY_H


#include "RenderSystemChild.h"


namespace LLGL
{


/**
\breif Sampler container interface.
\todo Maybe rename this to "SamplerHeap".
\see RenderSystem::CreateSamplerArray
*/
class LLGL_EXPORT SamplerArray : public RenderSystemChild { };


} // /namespace LLGL


#endif



// ================================================================================
