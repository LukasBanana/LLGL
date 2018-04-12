/*
 * VKSamplerArray.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_VK_SAMPLER_ARRAY_H
#define LLGL_VK_SAMPLER_ARRAY_H


#include <LLGL/SamplerArray.h>
#include <vulkan/vulkan.h>
#include <vector>


namespace LLGL
{


class Sampler;

class VKSamplerArray : public SamplerArray
{

    public:

        VKSamplerArray(std::uint32_t numSamplers, Sampler* const * samplerArray);

        //! Returns the array of Vulkan samplers.
        inline const std::vector<VkSampler>& GetSamplers() const
        {
            return samplers_;
        }

    private:

        std::vector<VkSampler> samplers_;

};


} // /namespace LLGL


#endif



// ================================================================================
