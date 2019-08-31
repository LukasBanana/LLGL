/*
 * VKSampler.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
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

        VKSampler(const VKPtr<VkDevice>& device, const SamplerDescriptor& desc);

        // Returns the Vulkan sampler object.
        inline VkSampler GetVkSampler() const
        {
            return sampler_.Get();
        }

    private:

        VKPtr<VkSampler> sampler_;

};


} // /namespace LLGL


#endif



// ================================================================================
