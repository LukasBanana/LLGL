/*
 * PipelineCache.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_PIPELINE_CACHE_H
#define LLGL_PIPELINE_CACHE_H


#include <LLGL/RenderSystemChild.h>
#include <LLGL/Blob.h>


namespace LLGL
{


/**
\brief Pipeline state cache interface.
\see RenderSystem::CreatePipelineCache
\see RenderSystem::CreatePipelineState
*/
class LLGL_EXPORT PipelineCache : public RenderSystemChild
{

        LLGL_DECLARE_INTERFACE( InterfaceID::PipelineCache );

    public:

        /**
        \brief Returns the cached blob representing a pipeline state.
        \remarks This blob can be saved to file and reused to speedup PSO creation on next application launch or reused during the same application run.
        If the backend does not support pipeline caching, the return value is an empty blob.
        */
        virtual Blob GetBlob() const = 0;

};


} // /namespace LLGL


#endif



// ================================================================================
