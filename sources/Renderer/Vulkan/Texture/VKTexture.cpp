/*
 * VKTexture.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "VKTexture.h"
#include "../Memory/VKDeviceMemory.h"
#include "../../TextureUtils.h"
#include "../../../Core/CoreUtils.h"
#include "../VKTypes.h"
#include "../VKCore.h"
#include <algorithm>


namespace LLGL
{


VKTexture::VKTexture(
    VkDevice                    device,
    VKDeviceMemoryManager&      deviceMemoryMngr,
    const TextureDescriptor&    desc)
:
    Texture    { desc.type, desc.bindFlags  },
    image_     { device                     },
    imageView_ { device, vkDestroyImageView },
    format_    { VKTypes::Map(desc.format)  }
{
    /* Create Vulkan image and allocate memory region */
    CreateImage(device, desc);
    image_.AllocateMemoryRegion(deviceMemoryMngr);
}

Extent3D VKTexture::GetMipExtent(std::uint32_t mipLevel) const
{
    switch (GetType())
    {
        case TextureType::Texture1D:
        case TextureType::Texture1DArray:
            return
            {
                std::max(1u, extent_.width  >> mipLevel),
                numArrayLayers_,
                1u
            };
        case TextureType::Texture2D:
        case TextureType::Texture2DArray:
        case TextureType::TextureCube:
        case TextureType::TextureCubeArray:
        case TextureType::Texture2DMS:
        case TextureType::Texture2DMSArray:
            return
            {
                std::max(1u, extent_.width  >> mipLevel),
                std::max(1u, extent_.height >> mipLevel),
                numArrayLayers_
            };
        case TextureType::Texture3D:
            return
            {
                std::max(1u, extent_.width  >> mipLevel),
                std::max(1u, extent_.height >> mipLevel),
                std::max(1u, extent_.depth  >> mipLevel)
            };
    }
    return { 0u, 0u, 0u };
}

TextureDescriptor VKTexture::GetDesc() const
{
    TextureDescriptor texDesc;

    texDesc.type        = GetType();
    texDesc.bindFlags   = GetBindFlags();
    texDesc.miscFlags   = 0;
    texDesc.format      = GetFormat();
    texDesc.arrayLayers = GetNumArrayLayers();
    texDesc.mipLevels   = GetNumMipLevels();

    switch (texDesc.type)
    {
        case TextureType::Texture1D:
        case TextureType::Texture1DArray:
            texDesc.extent.width    = extent_.width;
            texDesc.extent.height   = 1u;
            texDesc.extent.depth    = 1u;
            break;

        case TextureType::Texture2D:
        case TextureType::Texture2DArray:
            texDesc.extent.width    = extent_.width;
            texDesc.extent.height   = extent_.height;
            texDesc.extent.depth    = 1u;
            break;

        case TextureType::Texture3D:
            texDesc.extent.width    = extent_.width;
            texDesc.extent.height   = extent_.height;
            texDesc.extent.depth    = extent_.depth;
            break;

        case TextureType::TextureCube:
        case TextureType::TextureCubeArray:
            texDesc.extent.width    = extent_.width;
            texDesc.extent.height   = extent_.height;
            texDesc.extent.depth    = 1u;
            break;

        case TextureType::Texture2DMS:
        case TextureType::Texture2DMSArray:
            texDesc.extent.width    = extent_.width;
            texDesc.extent.height   = extent_.height;
            texDesc.extent.depth    = 1u;
            texDesc.samples         = static_cast<std::uint32_t>(sampleCountBits_);
            texDesc.miscFlags       |= MiscFlags::FixedSamples;
            break;
    }

    return texDesc;
}

Format VKTexture::GetFormat() const
{
    return VKTypes::Unmap(GetVkFormat());
}

SubresourceFootprint VKTexture::GetSubresourceFootprint(std::uint32_t mipLevel) const
{
    const Extent3D extent{ extent_.width, extent_.height, extent_.depth };
    SubresourceFootprint footprint = CalcPackedSubresourceFootprint(GetType(), GetFormat(), extent, mipLevel, GetNumArrayLayers());
    footprint.size = GetAlignedSize(footprint.size, static_cast<std::uint64_t>(image_.GetMemoryRequirements().alignment));
    return footprint;
}

void VKTexture::CreateImageView(
    VkDevice                    device,
    const TextureSubresource&   subresource,
    Format                      format,
    VKPtr<VkImageView>&         outImageView)
{
    VkImageSubresourceRange subresourceRange;
    {
        subresourceRange.aspectMask     = GetAspectFlags();
        subresourceRange.baseMipLevel   = subresource.baseMipLevel;
        subresourceRange.levelCount     = subresource.numMipLevels;
        subresourceRange.baseArrayLayer = subresource.baseArrayLayer;
        subresourceRange.layerCount     = subresource.numArrayLayers;
    }
    image_.CreateVkImageView(device, VKTypes::Map(GetType()), VKTypes::Map(format), subresourceRange, outImageView);
}

void VKTexture::CreateImageView(
    VkDevice                        device,
    const TextureViewDescriptor&    textureViewDesc,
    VKPtr<VkImageView>&             outImageView)
{
    VkImageSubresourceRange subresourceRange;
    {
        subresourceRange.aspectMask     = GetAspectFlags();
        subresourceRange.baseMipLevel   = textureViewDesc.subresource.baseMipLevel,
        subresourceRange.levelCount     = textureViewDesc.subresource.numMipLevels;
        subresourceRange.baseArrayLayer = textureViewDesc.subresource.baseArrayLayer;
        subresourceRange.layerCount     = textureViewDesc.subresource.numArrayLayers;
    }
    if (IsTextureSwizzleIdentity(textureViewDesc.swizzle))
    {
        image_.CreateVkImageView(
            device,
            VKTypes::Map(textureViewDesc.type),
            VKTypes::Map(textureViewDesc.format),
            subresourceRange,
            outImageView
        );
    }
    else
    {
        VkComponentMapping components;
        {
            components.r = VKTypes::ToVkComponentSwizzle(textureViewDesc.swizzle.r);
            components.g = VKTypes::ToVkComponentSwizzle(textureViewDesc.swizzle.g);
            components.b = VKTypes::ToVkComponentSwizzle(textureViewDesc.swizzle.b);
            components.a = VKTypes::ToVkComponentSwizzle(textureViewDesc.swizzle.a);
        }
        image_.CreateVkImageView(
            device,
            VKTypes::Map(textureViewDesc.type),
            VKTypes::Map(textureViewDesc.format),
            subresourceRange,
            outImageView,
            &components
        );
    }
}

void VKTexture::CreateInternalImageView(VkDevice device)
{
    VkImageSubresourceRange subresourceRange;
    {
        subresourceRange.aspectMask     = GetAspectFlags();
        subresourceRange.baseMipLevel   = 0;
        subresourceRange.levelCount     = GetNumMipLevels();
        subresourceRange.baseArrayLayer = 0;
        subresourceRange.layerCount     = GetNumArrayLayers();
    }
    image_.CreateVkImageView(device, VKTypes::Map(GetType()), format_, subresourceRange, imageView_);
}

static VkImageAspectFlags GetAspectFlagsByFormat(VkFormat format)
{
    switch (format)
    {
        case VK_FORMAT_D16_UNORM:
        case VK_FORMAT_X8_D24_UNORM_PACK32:
        case VK_FORMAT_D32_SFLOAT:
            return VK_IMAGE_ASPECT_DEPTH_BIT;
        case VK_FORMAT_S8_UINT:
            return VK_IMAGE_ASPECT_STENCIL_BIT;
        case VK_FORMAT_D16_UNORM_S8_UINT:
        case VK_FORMAT_D24_UNORM_S8_UINT:
        case VK_FORMAT_D32_SFLOAT_S8_UINT:
            return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        default:
            return VK_IMAGE_ASPECT_COLOR_BIT;
    }
}

VkImageAspectFlags VKTexture::GetAspectFlags() const
{
    return GetAspectFlagsByFormat(format_);
}


/*
 * ======= Private: =======
 */

// see https://www.khronos.org/registry/vulkan/specs/1.1-extensions/html/vkspec.html#resources-image-views-compatibility
static VkImageCreateFlags GetVkImageCreateFlags(const TextureDescriptor& desc)
{
    VkImageCreateFlags createFlags = 0;

    /*
    We only use VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT at the moment, to support cube maps.
    VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT is only required to make 3D textures compatible with 2D-array views, which LLGL does not support.
    */
    switch (desc.type)
    {
        case TextureType::TextureCube:
        case TextureType::TextureCubeArray:
            createFlags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
            break;
        default:
            break;
    }

    return createFlags;
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
            extent.width    = std::max(1u, desc.extent.width);
            extent.height   = 1u;
            extent.depth    = 1u;
            break;

        case VK_IMAGE_TYPE_2D:
            if (IsCubeTexture(desc.type))
            {
                /* Width and height must be equal for cube textures in Vulkan */
                extent.width    = std::max(1u, std::max(desc.extent.width, desc.extent.height));
                extent.height   = extent.width;
            }
            else
            {
                extent.width    = std::max(1u, desc.extent.width);
                extent.height   = std::max(1u, desc.extent.height);
            }
            extent.depth    = 1u;
            break;

        case VK_IMAGE_TYPE_3D:
            extent.width    = std::max(1u, desc.extent.width);
            extent.height   = std::max(1u, desc.extent.height);
            extent.depth    = std::max(1u, desc.extent.depth);
            break;

        default:
            extent.width    = 1u;
            extent.height   = 1u;
            extent.depth    = 1u;
            break;
    }

    return extent;
}

static std::uint32_t GetVkImageArrayLayers(const TextureDescriptor& desc, const VkImageType imageType)
{
    switch (imageType)
    {
        case VK_IMAGE_TYPE_1D:
        case VK_IMAGE_TYPE_2D:
            return std::max(1u, desc.arrayLayers);

        default:
            return 1u;
    }
}

//TODO:
//returned value must be a bit value from "VkImageFormatProperties::sampleCounts"
//that was returned by "vkGetPhysicalDeviceImageFormatProperties"
static VkSampleCountFlagBits GetVkImageSampleCountFlags(const TextureDescriptor& desc)
{
    if (IsMultiSampleTexture(desc.type))
        return VKTypes::ToVkSampleCountBits(desc.samples);
    else
        return VK_SAMPLE_COUNT_1_BIT;
}

static VkImageUsageFlags GetVkImageUsageFlags(const TextureDescriptor& desc)
{
    VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    /* Enable TRANSFER_SRC_BIT image usage when MIP-maps are enabled */
    if (IsMipMappedTexture(desc) || (desc.bindFlags & BindFlags::CopySrc) != 0)
        usageFlags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

    /* Enable either color or depth-stencil ATTACHMENT_BIT image usage when attachment usage is enabled */
    if ((desc.bindFlags & BindFlags::ColorAttachment) != 0)
        usageFlags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    else if ((desc.bindFlags & BindFlags::DepthStencilAttachment) != 0)
        usageFlags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;

    /* Enable sampling the image */
    if ((desc.bindFlags & BindFlags::Sampled) != 0)
        usageFlags |= VK_IMAGE_USAGE_SAMPLED_BIT;

    /* Enable load/store operations on the image */
    if ((desc.bindFlags & BindFlags::Storage) != 0)
        usageFlags |= VK_IMAGE_USAGE_STORAGE_BIT;

    #if 0//???
    /* Enable input attachment bit when used for reading AND as attachment */
    if ( (desc.bindFlags & (BindFlags::Sampled         | BindFlags::Storage               )) != 0 &&
         (desc.bindFlags & (BindFlags::ColorAttachment | BindFlags::DepthStencilAttachment)) != 0 )
    {
        usageFlags |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
    }
    #endif

    return usageFlags;
}

void VKTexture::CreateImage(VkDevice device, const TextureDescriptor& desc)
{
    /* Setup texture parameters */
    VkImageType imageType = GetVkImageType(desc.type);

    extent_             = GetVkImageExtent3D(desc, imageType);
    numMipLevels_       = NumMipLevels(desc);
    numArrayLayers_     = GetVkImageArrayLayers(desc, imageType);
    sampleCountBits_    = GetVkImageSampleCountFlags(desc);

    /* Create image object */
    image_.CreateVkImage(
        device,
        imageType,
        format_,
        extent_,
        numMipLevels_,
        numArrayLayers_,
        GetVkImageCreateFlags(desc),
        sampleCountBits_,
        GetVkImageUsageFlags(desc)
    );
}


} // /namespace LLGL



// ================================================================================
