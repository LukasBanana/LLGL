/*
 * VKTexture.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2017 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKTexture.h"
#include "../Memory/VKDeviceMemory.h"
#include "../VKTypes.h"
#include "../VKCore.h"
#include <algorithm>


namespace LLGL
{


static VkImageCreateFlags GetVkImageCreateFlags(const TextureDescriptor& desc)
{
    VkImageCreateFlags flags = 0;

    if (IsCubeTexture(desc.type))
        flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

    return flags;
}

static VkExtent3D GetVkImageExtent3D(const TextureDescriptor& desc, const VkImageType imageType)
{
    VkExtent3D extent;

    switch (imageType)
    {
        case VK_IMAGE_TYPE_1D:
            extent.width    = std::max(1u, desc.texture1D.width);
            extent.height   = 1u;
            extent.depth    = 1u;
            break;

        case VK_IMAGE_TYPE_2D:
            if (IsCubeTexture(desc.type))
            {
                /* Width and height must be equal for cube textures in Vulkan */
                extent.width    = std::max(1u, std::max(desc.textureCube.width, desc.textureCube.height));
                extent.height   = extent.width;
            }
            else
            {
                extent.width    = std::max(1u, desc.texture2D.width);
                extent.height   = std::max(1u, desc.texture2D.height);
            }
            extent.depth    = 1u;
            break;

        case VK_IMAGE_TYPE_3D:
            extent.width    = std::max(1u, desc.texture3D.width);
            extent.height   = std::max(1u, desc.texture3D.height);
            extent.depth    = std::max(1u, desc.texture3D.depth);
            break;

        default:
            extent.width    = 1u;
            extent.height   = 1u;
            extent.depth    = 1u;
            break;
    }

    return extent;
}

static std::uint32_t GetVkImageMipLevels(const TextureDescriptor& desc, const VkExtent3D& extent)
{
    if (!IsMultiSampleTexture(desc.type) && (desc.flags & TextureFlags::GenerateMips) != 0)
        return NumMipLevels(extent.width, extent.height, extent.depth);
    else
        return 1u;
}

static std::uint32_t GetVkImageArrayLayers(const TextureDescriptor& desc, const VkImageType imageType)
{
    switch (imageType)
    {
        case VK_IMAGE_TYPE_1D:
            return std::max(1u, desc.texture1D.layers);

        case VK_IMAGE_TYPE_2D:
            if (IsCubeTexture(desc.type))
                return std::max(1u, desc.textureCube.layers) * 6;
            else
                return std::max(1u, desc.texture2D.layers);

        default:
            return 1u;
    }
}

static VkSampleCountFlagBits GetVkImageSampleCountFlags(const TextureDescriptor& desc)
{
    if (IsMultiSampleTexture(desc.type))
    {
        //TODO:
        //returned value must be a bit value from "VkImageFormatProperties::sampleCounts"
        //that was returned by "vkGetPhysicalDeviceImageFormatProperties"
    }
    return VK_SAMPLE_COUNT_1_BIT;
}

VKTexture::VKTexture(const VKPtr<VkDevice>& device, const TextureDescriptor& desc) :
    Texture { desc.type              },
    image_  { device, vkDestroyImage }
{
    /* Create sampler state */
    VkImageCreateInfo createInfo;
    {
        createInfo.sType                    = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        createInfo.pNext                    = nullptr;
        createInfo.flags                    = GetVkImageCreateFlags(desc);
        createInfo.imageType                = VKTypes::Map(desc.type);
        createInfo.format                   = VKTypes::Map(desc.format);
        createInfo.extent                   = GetVkImageExtent3D(desc, createInfo.imageType);
        createInfo.mipLevels                = GetVkImageMipLevels(desc, createInfo.extent);
        createInfo.arrayLayers              = GetVkImageArrayLayers(desc, createInfo.imageType);
        createInfo.samples                  = GetVkImageSampleCountFlags(desc);
        createInfo.tiling                   = VK_IMAGE_TILING_OPTIMAL;
        createInfo.usage                    = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
        createInfo.sharingMode              = VK_SHARING_MODE_EXCLUSIVE; // only used by graphics queue
        createInfo.queueFamilyIndexCount    = 0;
        createInfo.pQueueFamilyIndices      = nullptr;
        createInfo.initialLayout            = VK_IMAGE_LAYOUT_UNDEFINED;
    }
    VkResult result = vkCreateImage(device, &createInfo, nullptr, image_.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Vulkan image");
}

Gs::Vector3ui VKTexture::QueryMipLevelSize(std::uint32_t mipLevel) const
{
    //todo...
    return { 0, 0, 0 };
}

TextureDescriptor VKTexture::QueryDesc() const
{
    TextureDescriptor desc;

    desc.type = GetType();

    //todo...
    
    return desc;
}

void VKTexture::BindToMemory(VkDevice device, VKDeviceMemoryRegion* memoryRegion)
{
    if (memoryRegion)
    {
        memoryRegion_ = memoryRegion;
        memoryRegion_->BindImage(device, GetVkImage());
    }
}


} // /namespace LLGL



// ================================================================================
