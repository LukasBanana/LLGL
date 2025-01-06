/*
 * VKSampler.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "VKSampler.h"
#include "../VKTypes.h"
#include "../VKCore.h"
#include "../../ResourceUtils.h"
#include "../../../Core/CoreUtils.h"
#include <LLGL/Backend/Vulkan/NativeHandle.h>


namespace LLGL
{


VKSampler::VKSampler(VkDevice device, const SamplerDescriptor& desc) :
    sampler_ { VKSampler::CreateVkSampler(device, desc) }
{
}

bool VKSampler::GetNativeHandle(void* nativeHandle, std::size_t nativeHandleSize)
{
    if (auto* nativeHandleVK = GetTypedNativeHandle<Vulkan::ResourceNativeHandle>(nativeHandle, nativeHandleSize))
    {
        nativeHandleVK->type            = Vulkan::ResourceNativeType::Sampler;
        nativeHandleVK->sampler.sampler = GetVkSampler();
        return true;
    }
    return false;
}

static VkFilter GetVkFilter(const SamplerFilter filter)
{
    return (filter == SamplerFilter::Linear ? VK_FILTER_LINEAR : VK_FILTER_NEAREST);
}

static VkSamplerMipmapMode GetVkSamplerMipmapMode(const SamplerFilter filter)
{
    return (filter == SamplerFilter::Linear ? VK_SAMPLER_MIPMAP_MODE_LINEAR : VK_SAMPLER_MIPMAP_MODE_NEAREST);
}

static VkBorderColor GetVkBorderColor(const float (&color)[4])
{
    switch (GetStaticSamplerBorderColor(color))
    {
        default:
        case StaticSamplerBorderColor::TransparentBlack:    return VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
        case StaticSamplerBorderColor::OpaqueBlack:         return VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
        case StaticSamplerBorderColor::OpaqueWhite:         return VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    }
}

void VKSampler::ConvertDesc(VkSamplerCreateInfo& outDesc, const SamplerDescriptor& inDesc)
{
    outDesc.sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    outDesc.pNext                   = nullptr;
    outDesc.flags                   = 0;
    outDesc.magFilter               = GetVkFilter(inDesc.magFilter);
    outDesc.minFilter               = GetVkFilter(inDesc.minFilter);
    outDesc.addressModeU            = VKTypes::Map(inDesc.addressModeU);
    outDesc.addressModeV            = VKTypes::Map(inDesc.addressModeV);
    outDesc.addressModeW            = VKTypes::Map(inDesc.addressModeW);
    outDesc.mipLodBias              = inDesc.mipMapLODBias;
    outDesc.anisotropyEnable        = VKBoolean(inDesc.maxAnisotropy > 1);
    outDesc.maxAnisotropy           = static_cast<float>(inDesc.maxAnisotropy);
    outDesc.compareEnable           = VKBoolean(inDesc.compareEnabled);
    outDesc.compareOp               = VKTypes::Map(inDesc.compareOp);
    outDesc.borderColor             = GetVkBorderColor(inDesc.borderColor);
    outDesc.unnormalizedCoordinates = VK_FALSE;

    if (inDesc.mipMapEnabled)
    {
        outDesc.mipmapMode  = GetVkSamplerMipmapMode(inDesc.mipMapFilter);
        outDesc.minLod      = inDesc.minLOD;
        outDesc.maxLod      = inDesc.maxLOD;
    }
    else
    {
        /*
        Set MIP-map mode to fixed values.
        see https://www.khronos.org/registry/vulkan/specs/1.0/man/html/VkSamplerCreateInfo.html
        */
        outDesc.mipmapMode  = VK_SAMPLER_MIPMAP_MODE_NEAREST;
        outDesc.minLod      = 0.0f;
        outDesc.maxLod      = 0.25f;
    }
}

VKPtr<VkSampler> VKSampler::CreateVkSampler(VkDevice device, const SamplerDescriptor& desc)
{
    /* Create sampler state */
    VKPtr<VkSampler> sampler{ device, vkDestroySampler };
    {
        VkSamplerCreateInfo createInfo;
        VKSampler::ConvertDesc(createInfo, desc);
        VkResult result = vkCreateSampler(device, &createInfo, nullptr, sampler.ReleaseAndGetAddressOf());
        VKThrowIfFailed(result, "failed to create Vulkan sampler");
    }
    return sampler;
}


} // /namespace LLGL



// ================================================================================
