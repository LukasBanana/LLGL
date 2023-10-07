/*
 * ProxyPipelineCache.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "ProxyPipelineCache.h"
#include "CheckedCast.h"
#include "../Core/Assertion.h"


namespace LLGL
{


Blob ProxyPipelineCache::GetBlob() const
{
    return {}; // dummy
}

unsigned ProxyPipelineCache::Retain()
{
    return ++refCount_;
}

unsigned ProxyPipelineCache::Release()
{
    LLGL_ASSERT(refCount_ > 0);
    return --refCount_;
}

PipelineCache* ProxyPipelineCache::CreateInstance(HWObjectInstance<ProxyPipelineCache>& proxy)
{
    /* Return single instance of the proxy pipeline cache */
    if (proxy)
        proxy->Retain();
    else
        proxy = MakeUnique<ProxyPipelineCache>();
    return proxy.get();
}

void ProxyPipelineCache::ReleaseInstance(HWObjectInstance<ProxyPipelineCache>& proxy, PipelineCache& pipelineCache)
{
    /* Release single instance of the proxy pipeline cache */
    auto& pipelineCacheProxy = LLGL_CAST(ProxyPipelineCache&, pipelineCache);
    if (pipelineCacheProxy.Release() == 0)
    {
        if (proxy.get() == &pipelineCacheProxy)
            proxy.reset();
    }
}


} // /namespace LLGL



// ================================================================================
