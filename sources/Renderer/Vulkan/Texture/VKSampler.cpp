/*
 * VKSampler.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKSampler.h"
#include "../VKTypes.h"
#include "../VKCore.h"


namespace LLGL
{


static VkFilter GetMinMagFilter(const TextureFilter filter)
{
    return (filter == TextureFilter::Linear ? VK_FILTER_LINEAR : VK_FILTER_NEAREST);
}

static VkSamplerMipmapMode GetMipMapMode(const TextureFilter filter)
{
    return (filter == TextureFilter::Linear ? VK_SAMPLER_MIPMAP_MODE_LINEAR : VK_SAMPLER_MIPMAP_MODE_NEAREST);
}

static VkBorderColor GetBorderColor(const ColorRGBAf& color)
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
    /* Initialize sampler state descriptor */
    VkSamplerCreateInfo createInfo;

    createInfo.sType                    = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    createInfo.pNext                    = nullptr;
    createInfo.flags                    = 0;
    createInfo.magFilter                = GetMinMagFilter(desc.magFilter);
    createInfo.minFilter                = GetMinMagFilter(desc.minFilter);
    createInfo.mipmapMode               = GetMipMapMode(desc.mipMapFilter);
    createInfo.addressModeU             = VKTypes::Map(desc.textureWrapU);
    createInfo.addressModeV             = VKTypes::Map(desc.textureWrapV);
    createInfo.addressModeW             = VKTypes::Map(desc.textureWrapW);
    createInfo.mipLodBias               = desc.mipMapLODBias;
    createInfo.anisotropyEnable         = VKBoolean(desc.maxAnisotropy > 1);
    createInfo.maxAnisotropy            = static_cast<float>(desc.maxAnisotropy);
    createInfo.compareEnable            = VKBoolean(desc.compareEnabled);
    createInfo.compareOp                = VKTypes::Map(desc.compareOp);
    createInfo.minLod                   = desc.minLOD;
    createInfo.maxLod                   = desc.maxLOD;
    createInfo.borderColor              = GetBorderColor(desc.borderColor);
    createInfo.unnormalizedCoordinates  = VK_FALSE;

    /* Create sampler state */
    VkResult result = vkCreateSampler(device, &createInfo, nullptr, sampler_.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Vulkan sampler");
}


} // /namespace LLGL



// ================================================================================
