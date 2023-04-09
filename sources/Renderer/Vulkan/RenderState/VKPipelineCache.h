/*
 * VKPipelineCache.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_VK_PIPELINE_CACHE_H
#define LLGL_VK_PIPELINE_CACHE_H


#include <vulkan/vulkan.h>
#include "../VKPtr.h"


namespace LLGL
{


class VKPipelineCache
{

    public:

        VKPipelineCache(
            VkDevice                            device,
            const VkPhysicalDeviceProperties&   physicalDeviceProperties,
            const void*                         initialData,
            std::size_t                         initialDataSize
        );

        // Returns the size (in bytes) of the serialized cache data.
        std::size_t GetDataSize() const;

        // Writes the serialized cache data to the output buffer <data>.
        void GetData(void* data, std::size_t dataSize) const;

        // Returns the native pipeline cache object.
        inline VkPipelineCache GetNative() const
        {
            return cache_.Get();
        }

    private:

        VkDevice                device_ = VK_NULL_HANDLE;
        VKPtr<VkPipelineCache>  cache_;

};


} // /namespace LLGL


#endif



// ================================================================================
