/*
 * VKPipelineCache.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_VK_PIPELINE_CACHE_H
#define LLGL_VK_PIPELINE_CACHE_H


#include <vulkan/vulkan.h>
#include <LLGL/PipelineCache.h>
#include "../VKPtr.h"


namespace LLGL
{


class VKPipelineCache final : public PipelineCache
{

    public:

        VKPipelineCache(VkDevice device, const Blob& initialBlob);

        Blob GetBlob() const override;

    public:

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
