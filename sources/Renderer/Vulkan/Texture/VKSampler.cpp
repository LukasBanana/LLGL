/*
 * VKSampler.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKSampler.h"
#include "../VKTypes.h"
#include "../VKCore.h"


namespace LLGL
{


static VkFilter GetVkFilter(const SamplerFilter filter)
{
    return (filter == SamplerFilter::Linear ? VK_FILTER_LINEAR : VK_FILTER_NEAREST);
}

static VkSamplerMipmapMode GetVkSamplerMipmapMode(const SamplerFilter filter)
{
    return (filter == SamplerFilter::Linear ? VK_SAMPLER_MIPMAP_MODE_LINEAR : VK_SAMPLER_MIPMAP_MODE_NEAREST);
}

static VkBorderColor GetVkBorderColor(const ColorRGBAf& color)
{
    if (color.a > 0.5f)
    {
        if (color.r <= 0.5f && color.g <= 0.5f && color.b <= 0.5f)
            return VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
        if (color.r > 0.5f && color.g > 0.5f && color.b > 0.5f)
            return VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    }
    return VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
}

VKSampler::VKSampler(const VKPtr<VkDevice>& device, const SamplerDescriptor& desc) :
    sampler_ { device, vkDestroySampler }
{
    /* Create sampler state */
    VkSamplerCreateInfo createInfo;
    {
        createInfo.sType                    = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        createInfo.pNext                    = nullptr;
        createInfo.flags                    = 0;
        createInfo.magFilter                = GetVkFilter(desc.magFilter);
        createInfo.minFilter                = GetVkFilter(desc.minFilter);
        createInfo.addressModeU             = VKTypes::Map(desc.addressModeU);
        createInfo.addressModeV             = VKTypes::Map(desc.addressModeV);
        createInfo.addressModeW             = VKTypes::Map(desc.addressModeW);
        createInfo.mipLodBias               = desc.mipMapLODBias;
        createInfo.anisotropyEnable         = VKBoolean(desc.maxAnisotropy > 1);
        createInfo.maxAnisotropy            = static_cast<float>(desc.maxAnisotropy);
        createInfo.compareEnable            = VKBoolean(desc.compareEnabled);
        createInfo.compareOp                = VKTypes::Map(desc.compareOp);
        createInfo.borderColor              = GetVkBorderColor(desc.borderColor);
        createInfo.unnormalizedCoordinates  = VK_FALSE;

        if (desc.mipMapping)
        {
            createInfo.mipmapMode   = GetVkSamplerMipmapMode(desc.mipMapFilter);
            createInfo.minLod       = desc.minLOD;
            createInfo.maxLod       = desc.maxLOD;
        }
        else
        {
            /*
            Set MIP-map mode to fixed values.
            see https://www.khronos.org/registry/vulkan/specs/1.0/man/html/VkSamplerCreateInfo.html
            */
            createInfo.mipmapMode   = VK_SAMPLER_MIPMAP_MODE_NEAREST;
            createInfo.minLod       = 0.0f;
            createInfo.maxLod       = 0.25f;
        }
    }
    VkResult result = vkCreateSampler(device, &createInfo, nullptr, sampler_.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Vulkan sampler");
}


} // /namespace LLGL



// ================================================================================
