/*
 * VKDeviceImage.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "VKDeviceImage.h"
#include "../Memory/VKDeviceMemory.h"
#include "../Memory/VKDeviceMemoryManager.h"
#include "../Command/VKCommandContext.h"
#include "../VKCore.h"
#include "../../../Core/Exception.h"
#include "../../../Core/PrintfUtils.h"


namespace LLGL
{


VKDeviceImage::VKDeviceImage(VkDevice device) :
    image_ { device, vkDestroyImage }
{
}

VKDeviceImage::VKDeviceImage(VKDeviceImage&& rhs) :
    image_              { std::move(rhs.image_)   },
    layout_             { rhs.layout_             },
    memoryRequirements_ { rhs.memoryRequirements_ },
    memoryRegion_       { rhs.memoryRegion_       }
{
}

VKDeviceImage& VKDeviceImage::operator = (VKDeviceImage&& rhs)
{
    image_              = std::move(rhs.image_);
    layout_             = rhs.layout_;
    memoryRequirements_ = rhs.memoryRequirements_;
    memoryRegion_       = rhs.memoryRegion_;
    return *this;
}

void VKDeviceImage::AllocateMemoryRegion(VKDeviceMemoryManager& deviceMemoryMngr)
{
    VkDevice device = deviceMemoryMngr.GetVkDevice();

    /* Get memory requirements for the image */
    vkGetImageMemoryRequirements(device, image_, &memoryRequirements_);

    /* Allocate device memory */
    memoryRegion_ = deviceMemoryMngr.Allocate(
        memoryRequirements_.size,
        memoryRequirements_.alignment,
        memoryRequirements_.memoryTypeBits,
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );

    /* Bind image to device memory region */
    if (memoryRegion_ == nullptr)
    {
        LLGL_TRAP(
            "failed to allocate 0x%016" PRIX64 " bytes of device memory with alignment 0x%016" PRIX64 " for Vulkan image",
            memoryRequirements_.size, memoryRequirements_.alignment
        );
    }

    memoryRegion_->BindImage(device, image_);
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
        createInfo.initialLayout            = VK_IMAGE_LAYOUT_UNDEFINED; // must be UNDEFINED or PREINITIALIZED
    }
    VkResult result = vkCreateImage(device, &createInfo, nullptr, image_.ReleaseAndGetAddressOf());
    VKThrowIfCreateFailed(result, "VkImage");
}

void VKDeviceImage::ReleaseVkImage()
{
    image_.Release();
}

void VKDeviceImage::CreateVkImageView(
    VkDevice                        device,
    VkImageViewType                 viewType,
    VkFormat                        format,
    const VkImageSubresourceRange&  subresourceRange,
    VKPtr<VkImageView>&             outImageView,
    const VkComponentMapping*       components)
{
    /* Create image view object */
    VkImageViewCreateInfo createInfo;
    {
        createInfo.sType            = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.pNext            = nullptr;
        createInfo.flags            = 0;
        createInfo.image            = image_;
        createInfo.viewType         = viewType;
        createInfo.format           = format;
        createInfo.subresourceRange = subresourceRange;

        if (components == nullptr)
        {
            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        }
        else
            createInfo.components = *components;
    }
    VkResult result = vkCreateImageView(device, &createInfo, nullptr, outImageView.ReleaseAndGetAddressOf());
    VKThrowIfCreateFailed(result, "VkImageView");
}

VkImageLayout VKDeviceImage::TransitionImageLayout(
    VKCommandContext&           context,
    VkFormat                    format,
    VkImageLayout               newLayout,
    const TextureSubresource&   subresource)
{
    VkImageLayout oldLayout = layout_;
    if (newLayout != oldLayout)
    {
        context.ImageMemoryBarrier(image_, format, oldLayout, newLayout, subresource);
        layout_ = newLayout;
    }
    return (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED ? layout_ : oldLayout);
}


} // /namespace LLGL



// ================================================================================
