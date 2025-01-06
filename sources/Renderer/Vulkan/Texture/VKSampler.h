/*
 * VKSampler.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_VK_SAMPLER_H
#define LLGL_VK_SAMPLER_H


#include <LLGL/Sampler.h>
#include <vulkan/vulkan.h>
#include "../VKPtr.h"


namespace LLGL
{


class VKSampler final : public Sampler
{

    public:

        #include <LLGL/Backend/Sampler.inl>

    public:

        VKSampler(VkDevice device, const SamplerDescriptor& desc);

        // Returns the Vulkan sampler object.
        inline VkSampler GetVkSampler() const
        {
            return sampler_.Get();
        }

    public:

        // Converts the specified sampler descriptor to the native Vulkan descriptor.
        static void ConvertDesc(VkSamplerCreateInfo& outDesc, const SamplerDescriptor& inDesc);

        // Creates a native Vulkan sampler.
        static VKPtr<VkSampler> CreateVkSampler(VkDevice device, const SamplerDescriptor& desc);

    private:

        VKPtr<VkSampler> sampler_;

};


} // /namespace LLGL


#endif



// ================================================================================
