/*
 * VKPipelineCache.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKPipelineCache.h"


namespace LLGL
{


// Structure for the cache data header
struct VKPipelineCacheDataHeader
{
    std::uint32_t length;
    std::uint32_t version;                          // VkPipelineCacheHeaderVersion (VK_PIPELINE_CACHE_HEADER_VERSION_ONE)
    std::uint32_t vendorID;                         // VkPhysicalDeviceProperties::vendorID
    std::uint32_t deviceID;                         // VkPhysicalDeviceProperties::deviceID
    std::uint8_t  pipelineCacheUUID[VK_UUID_SIZE];  // VkPhysicalDeviceProperties::pipelineCacheUUID
};

VKPipelineCache::VKPipelineCache(
    VkDevice                            device,
    const VkPhysicalDeviceProperties&   physicalDeviceProperties,
    const void*                         initialData,
    std::size_t                         initialDataSize)
:
    cache_ { device, vkDestroyPipelineCache }
{
    VkPipelineCacheCreateInfo createInfo;
    {
        createInfo.sType            = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
        createInfo.pNext            = nullptr;
        createInfo.flags            = 0;
        createInfo.initialDataSize  = initialDataSize;
        createInfo.pInitialData     = initialData;
    }
    vkCreatePipelineCache(device, &createInfo, nullptr, cache_.ReleaseAndGetAddressOf());
}

std::size_t VKPipelineCache::GetDataSize() const
{
    std::size_t dataSize = 0;
    vkGetPipelineCacheData(device_, cache_, &dataSize, nullptr);
    return dataSize;
}

void VKPipelineCache::GetData(void* data, std::size_t dataSize) const
{
    vkGetPipelineCacheData(device_, cache_, &dataSize, data);
}


} // /namespace LLGL



// ================================================================================
