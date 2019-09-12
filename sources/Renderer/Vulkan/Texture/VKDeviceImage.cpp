/*
 * VKDeviceImage.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKDeviceImage.h"
#include "../Memory/VKDeviceMemory.h"
#include "../Memory/VKDeviceMemoryManager.h"
#include "../VKCore.h"


namespace LLGL
{


VKDeviceImage::VKDeviceImage(const VKPtr<VkDevice>& device) :
    image_ { device, vkDestroyImage }
{
}

void VKDeviceImage::AllocateMemoryRegion(VKDeviceMemoryManager& deviceMemoryMngr)
{
    auto device = deviceMemoryMngr.GetVkDevice();

    /* Get memory requirements for the image */
    VkMemoryRequirements requirements;
    vkGetImageMemoryRequirements(device, image_, &requirements);

    /* Allocate device memory */
    memoryRegion_ = deviceMemoryMngr.Allocate(
        requirements.size,
        requirements.alignment,
        requirements.memoryTypeBits,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );

    /* Bind image to device memory region */
    if (memoryRegion_)
        memoryRegion_->BindImage(device, image_);
    else
        throw std::runtime_error("failed to allocate device memory for Vulkan image");
}

void VKDeviceImage::ReleaseMemoryRegion(VKDeviceMemoryManager& deviceMemoryMngr)
{
    deviceMemoryMngr.Release(memoryRegion_);
    memoryRegion_ = nullptr;
}

void VKDeviceImage::BindMemoryRegion(VkDevice device, VKDeviceMemoryRegion* memoryRegion)
{
    if (memoryRegion)
    {
        memoryRegion_ = memoryRegion;
        memoryRegion_->BindImage(device, GetVkImage());
    }
}

void VKDeviceImage::CreateVkImage(
    VkDevice                device,
    VkImageType             imageType,
    VkFormat                format,
    const VkExtent3D&       extent,
    std::uint32_t           numMipLevels,
    std::uint32_t           numArrayLayers,
    VkImageCreateFlags      createFlags,
    VkSampleCountFlagBits   sampleCountBits,
    VkImageUsageFlags       usageFlags)
{
    /* Create image object */
    VkImageCreateInfo createInfo;
    {
        createInfo.sType                    = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        createInfo.pNext                    = nullptr;
        createInfo.flags                    = createFlags;
        createInfo.imageType                = imageType;
        createInfo.format                   = format;
        createInfo.extent                   = extent;
        createInfo.mipLevels                = numMipLevels;
        createInfo.arrayLayers              = numArrayLayers;
        createInfo.samples                  = sampleCountBits;
        createInfo.tiling                   = VK_IMAGE_TILING_OPTIMAL;
        createInfo.usage                    = usageFlags;
        createInfo.sharingMode              = VK_SHARING_MODE_EXCLUSIVE; // only used by graphics queue
        createInfo.queueFamilyIndexCount    = 0;
        createInfo.pQueueFamilyIndices      = nullptr;
        createInfo.initialLayout            = VK_IMAGE_LAYOUT_UNDEFINED;
    }
    VkResult result = vkCreateImage(device, &createInfo, nullptr, image_.ReleaseAndGetAddressOf());
    VKThrowIfCreateFailed(result, "VkImage");
}

void VKDeviceImage::ReleaseVkImage()
{
    image_.Release();
}

void VKDeviceImage::CreateVkImageView(
    VkDevice            device,
    VkImageViewType     viewType,
    VkFormat            format,
    VkImageAspectFlags  aspectFlags,
    std::uint32_t       baseMipLevel,
    std::uint32_t       numMipLevels,
    std::uint32_t       baseArrayLayer,
    std::uint32_t       numArrayLayers,
    VkImageView*        imageViewRef)
{
    /* Create image view object */
    VkImageViewCreateInfo createInfo;
    {
        createInfo.sType                            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.pNext                            = nullptr;
        createInfo.flags                            = 0;
        createInfo.image                            = image_;
        createInfo.viewType                         = viewType;
        createInfo.format                           = format;
        createInfo.components.r                     = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g                     = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b                     = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a                     = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask      = aspectFlags;
        createInfo.subresourceRange.baseMipLevel    = baseMipLevel;
        createInfo.subresourceRange.levelCount      = numMipLevels;
        createInfo.subresourceRange.baseArrayLayer  = baseArrayLayer;
        createInfo.subresourceRange.layerCount      = numArrayLayers;
    }
    VkResult result = vkCreateImageView(device, &createInfo, nullptr, imageViewRef);
    VKThrowIfCreateFailed(result, "VkImageView");
}


} // /namespace LLGL



// ================================================================================
