/*
 * VKTexture.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKTexture.h"
#include "../Memory/VKDeviceMemory.h"
#include "../VKTypes.h"
#include "../VKCore.h"
#include <algorithm>


namespace LLGL
{


VKTexture::VKTexture(
    const VKPtr<VkDevice>& device, VKDeviceMemoryManager& deviceMemoryMngr, const TextureDescriptor& desc) :
        Texture       { desc.type                  },
        imageWrapper_ { device                     },
        imageView_    { device, vkDestroyImageView },
        format_       { VKTypes::Map(desc.format)  }
{
    /* Create Vulkan image and allocate memory region */
    CreateImage(device, desc);
    imageWrapper_.AllocateMemoryRegion(deviceMemoryMngr);
}

Extent3D VKTexture::QueryMipExtent(std::uint32_t mipLevel) const
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

TextureDescriptor VKTexture::QueryDesc() const
{
    TextureDescriptor texDesc;

    texDesc.type    = GetType();
    texDesc.format  = VKTypes::Unmap(GetVkFormat());
    texDesc.flags   = 0;

    switch (texDesc.type)
    {
        case TextureType::Texture1D:
        case TextureType::Texture1DArray:
            texDesc.extent.width    = extent_.width;
            texDesc.arrayLayers     = numArrayLayers_;
            break;

        case TextureType::Texture2D:
        case TextureType::Texture2DArray:
            texDesc.extent.width    = extent_.width;
            texDesc.extent.height   = extent_.height;
            texDesc.arrayLayers     = numArrayLayers_;
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
            texDesc.arrayLayers     = numArrayLayers_ / 6;
            break;

        case TextureType::Texture2DMS:
        case TextureType::Texture2DMSArray:
            texDesc.extent.width    = extent_.width;
            texDesc.extent.height   = extent_.height;
            texDesc.arrayLayers     = numArrayLayers_;
            texDesc.samples         = 0; //TODO
            texDesc.flags           |= TextureFlags::FixedSamples;
            break;
    }

    return texDesc;
}

void VKTexture::CreateImageView(
    VkDevice        device,
    std::uint32_t   baseMipLevel,
    std::uint32_t   numMipLevels,
    std::uint32_t   baseArrayLayer,
    std::uint32_t   numArrayLayers,
    VkImageView*    imageViewRef)
{
    imageWrapper_.CreateVkImageView(
        device,
        VKTypes::Map(GetType()),
        format_,
        VK_IMAGE_ASPECT_COLOR_BIT,
        baseMipLevel,
        numMipLevels,
        baseArrayLayer,
        numArrayLayers,
        imageViewRef
    );
}

void VKTexture::CreateInternalImageView(VkDevice device)
{
    CreateImageView(device, 0, GetNumMipLevels(), 0, GetNumArrayLayers(), imageView_.ReleaseAndGetAddressOf());
}


/*
 * ======= Private: =======
 */

static VkImageCreateFlags GetVkImageCreateFlags(const TextureDescriptor& desc)
{
    VkImageCreateFlags createFlags = 0;

    if (IsCubeTexture(desc.type))
        createFlags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    if (desc.type == TextureType::Texture2DArray || desc.type == TextureType::Texture2DMSArray)
        createFlags |= VK_IMAGE_CREATE_2D_ARRAY_COMPATIBLE_BIT_KHR;

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
            return std::max(1u, desc.arrayLayers);

        case VK_IMAGE_TYPE_2D:
            if (IsCubeTexture(desc.type))
                return std::max(1u, desc.arrayLayers) * 6;
            else
                return std::max(1u, desc.arrayLayers);

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
    VkImageUsageFlags usageFlags = VK_IMAGE_USAGE_TRANSFER_DST_BIT;

    /* Enable TRANSFER_SRC_BIT image usage when MIP-maps are enabled */
    if (IsMipMappedTexture(desc))
        usageFlags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;

    /* Enable either color or depth-stencil ATTACHMENT_BIT image usage when attachment usage is enabled */
    if ((desc.flags & TextureFlags::AttachmentUsage) != 0)
    {
        if (IsDepthStencilFormat(desc.format))
            usageFlags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        else
            usageFlags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }

    /* Enable sampling the image */
    if ((desc.flags & TextureFlags::SampleUsage) != 0)
        usageFlags |= VK_IMAGE_USAGE_SAMPLED_BIT;

    /* Enable load/store operations on the image */
    if ((desc.flags & TextureFlags::StorageUsage) != 0)
        usageFlags |= VK_IMAGE_USAGE_STORAGE_BIT;

    return usageFlags;
}

void VKTexture::CreateImage(VkDevice device, const TextureDescriptor& desc)
{
    /* Setup texture parameters */
    auto imageType  = GetVkImageType(desc.type);

    extent_         = GetVkImageExtent3D(desc, imageType);
    numMipLevels_   = NumMipLevels(desc);
    numArrayLayers_ = GetVkImageArrayLayers(desc, imageType);

    /* Create image object */
    imageWrapper_.CreateVkImage(
        device,
        imageType,
        format_,
        extent_,
        numMipLevels_,
        numArrayLayers_,
        GetVkImageCreateFlags(desc),
        GetVkImageSampleCountFlags(desc),
        GetVkImageUsageFlags(desc)
    );
}


} // /namespace LLGL



// ================================================================================
