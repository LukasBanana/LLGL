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


VKTexture::VKTexture(const VKPtr<VkDevice>& device, const TextureDescriptor& desc) :
    Texture    { desc.type                  },
    image_     { device, vkDestroyImage     },
    imageView_ { device, vkDestroyImageView },
    format_    { VKTypes::Map(desc.format)  }
{
    CreateImage(device, desc);
}

Gs::Vector3ui VKTexture::QueryMipLevelSize(std::uint32_t mipLevel) const
{
    return Gs::Vector3ui
    {
        std::max(1u, extent_.width  >> mipLevel),
        std::max(1u, extent_.height >> mipLevel),
        std::max(1u, extent_.depth  >> mipLevel)
    };
}

TextureDescriptor VKTexture::QueryDesc() const
{
    TextureDescriptor desc;

    desc.type   = GetType();
    //desc.format = ;

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

void VKTexture::CreateImageView(VkDevice device, std::uint32_t baseArrayLayer, std::uint32_t baseMipLevel, std::uint32_t numMipLevels, VkImageView* imageViewRef)
{
    /* Create image view object */
    VkImageViewCreateInfo createInfo;
    {
        createInfo.sType                            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.pNext                            = nullptr;
        createInfo.flags                            = 0;
        createInfo.image                            = image_;
        createInfo.viewType                         = VKTypes::Map(GetType());
        createInfo.format                           = format_;
        createInfo.components.r                     = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g                     = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b                     = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a                     = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask      = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel    = baseMipLevel;
        createInfo.subresourceRange.levelCount      = numMipLevels;
        createInfo.subresourceRange.baseArrayLayer  = baseArrayLayer;
        createInfo.subresourceRange.layerCount      = 1;
    }
    VkResult result = vkCreateImageView(device, &createInfo, nullptr, imageViewRef);
    VKThrowIfFailed(result, "failed to create Vulkan image view");
}

void VKTexture::CreateInternalImageView(VkDevice device)
{
    CreateImageView(device, 0, 0, GetNumMipLevels(), imageView_.ReleaseAndGetAddressOf());
}


/*
 * ======= Private: =======
 */

static VkImageCreateFlags GetVkImageCreateFlags(const TextureDescriptor& desc)
{
    VkImageCreateFlags flags = 0;

    if (IsCubeTexture(desc.type))
        flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

    return flags;
}

static VkImageType GetVkImageType(const TextureType textureType)
{
    if (textureType == TextureType::Texture3D)
        return VK_IMAGE_TYPE_3D;
    if (textureType == TextureType::Texture1D || textureType == TextureType::Texture1DArray)
        return VK_IMAGE_TYPE_1D;
    return VK_IMAGE_TYPE_2D;
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

static bool HasTextureMipMaps(const TextureDescriptor& desc)
{
    return (!IsMultiSampleTexture(desc.type) && (desc.flags & TextureFlags::GenerateMips) != 0);
}

static std::uint32_t GetVkImageMipLevels(const TextureDescriptor& desc, const VkExtent3D& extent)
{
    if (HasTextureMipMaps(desc))
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

static VkImageUsageFlags GetVkImageUsageFlags(const TextureDescriptor& desc)
{
    VkImageUsageFlags flags = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;

    /* Enable TRANSFER_SRC_BIT image usage when MIP-maps are enabled */
    if (HasTextureMipMaps(desc))
        flags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

    /* Enable either color or depth-stencil ATTACHMENT_BIT image usage when attachment usage is enabled */
    if ((desc.flags & TextureFlags::AttachmentUsage) != 0)
    {
        if (IsDepthStencilFormat(desc.format))
            flags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        else
            flags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }

    return flags;
}

void VKTexture::CreateImage(VkDevice device, const TextureDescriptor& desc)
{
    /* Create image object */
    VkImageCreateInfo createInfo;
    {
        createInfo.sType                    = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        createInfo.pNext                    = nullptr;
        createInfo.flags                    = GetVkImageCreateFlags(desc);
        createInfo.imageType                = GetVkImageType(desc.type);
        createInfo.format                   = format_;
        createInfo.extent                   = GetVkImageExtent3D(desc, createInfo.imageType);
        createInfo.mipLevels                = GetVkImageMipLevels(desc, createInfo.extent);
        createInfo.arrayLayers              = GetVkImageArrayLayers(desc, createInfo.imageType);
        createInfo.samples                  = GetVkImageSampleCountFlags(desc);
        createInfo.tiling                   = VK_IMAGE_TILING_OPTIMAL;
        createInfo.usage                    = GetVkImageUsageFlags(desc);
        createInfo.sharingMode              = VK_SHARING_MODE_EXCLUSIVE; // only used by graphics queue
        createInfo.queueFamilyIndexCount    = 0;
        createInfo.pQueueFamilyIndices      = nullptr;
        createInfo.initialLayout            = VK_IMAGE_LAYOUT_UNDEFINED;
    }
    VkResult result = vkCreateImage(device, &createInfo, nullptr, image_.ReleaseAndGetAddressOf());
    VKThrowIfFailed(result, "failed to create Vulkan image");

    /* Store number of MIP level and image extent */
    extent_         = createInfo.extent;
    numMipLevels_   = createInfo.mipLevels;
}


} // /namespace LLGL



// ================================================================================
