/*
 * VKPipelineCache.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "VKPipelineCache.h"
#include <cstdint>


namespace LLGL
{


VKPipelineCache::VKPipelineCache(VkDevice device, const Blob& initialBlob)
:
    device_ { device                         },
    cache_  { device, vkDestroyPipelineCache }
{
    VkPipelineCacheCreateInfo createInfo;
    {
        createInfo.sType            = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
        createInfo.pNext            = nullptr;
        createInfo.flags            = 0;
        createInfo.initialDataSize  = initialBlob.GetSize();
        createInfo.pInitialData     = initialBlob.GetData();
    }
    vkCreatePipelineCache(device, &createInfo, nullptr, cache_.ReleaseAndGetAddressOf());
}

Blob VKPipelineCache::GetBlob() const
{
    /* Determine cache size and return binary data */
    std::size_t dataSize = 0;
    vkGetPipelineCacheData(device_, cache_, &dataSize, nullptr);

    DynamicByteArray data{ dataSize, UninitializeTag{} };
    vkGetPipelineCacheData(device_, cache_, &dataSize, data.get());

    return Blob::CreateStrongRef(std::move(data));
}


} // /namespace LLGL



// ================================================================================
