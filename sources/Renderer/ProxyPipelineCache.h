/*
 * ProxyPipelineCache.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_PROXY_PIPELINE_CACHE_H
#define LLGL_PROXY_PIPELINE_CACHE_H


#include <LLGL/Export.h>
#include <LLGL/PipelineCache.h>
#include "ContainerTypes.h"
#include <atomic>


namespace LLGL
{


// Proxy implementation (placeholder) for backends that do not support pipeline caching.
class LLGL_EXPORT ProxyPipelineCache final : public PipelineCache
{

    public:

        Blob GetBlob() const override;

    public:

        // Increments the reference counter and returns the new value.
        unsigned Retain();

        // Decrements the reference counter and returns the new value. If this reaches zero, the object must be deleted.
        unsigned Release();

    public:

        // Default implementation to create a proxy pipeline cache of a single instance.
        static PipelineCache* CreateInstance(HWObjectInstance<ProxyPipelineCache>& proxy);

        // Default implementation to release the proxy pipeline cache of a single instance.
        static void ReleaseInstance(HWObjectInstance<ProxyPipelineCache>& proxy, PipelineCache& pipelineCache);

    private:

        std::atomic_uint refCount_ { 1 }; // Reference counter starting at 1.

};


} // /namespace LLGL


#endif



// ================================================================================
