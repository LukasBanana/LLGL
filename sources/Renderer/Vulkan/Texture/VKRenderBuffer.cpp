/*
 * VKRenderBuffer.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "VKRenderBuffer.h"
#include "../Memory/VKDeviceMemoryManager.h"


namespace LLGL
{


VKRenderBuffer::VKRenderBuffer(VkDevice device) :
    VKDeviceImage { device                     },
    imageView_    { device, vkDestroyImageView }
{
}

VKRenderBuffer::VKRenderBuffer(VKRenderBuffer&& rhs) :
    VKDeviceImage { std::move(rhs)            },
    imageView_    { std::move(rhs.imageView_) },
    format_       { rhs.format_               },
    memoryMngr_   { rhs.memoryMngr_           }
{
    rhs.format_     = VK_FORMAT_UNDEFINED;
    rhs.memoryMngr_ = nullptr;
}

VKRenderBuffer& VKRenderBuffer::operator = (VKRenderBuffer&& rhs)
{
    VKDeviceImage::operator=(std::move(rhs));
    this->imageView_    = std::move(rhs.imageView_);
    this->format_       = rhs.format_;
    this->memoryMngr_   = rhs.memoryMngr_;
    rhs.format_         = VK_FORMAT_UNDEFINED;
    rhs.memoryMngr_     = nullptr;
    return *this;
}

VKRenderBuffer::~VKRenderBuffer()
{
    Release();
}

void VKRenderBuffer::Create(
    VKDeviceMemoryManager&  deviceMemoryMngr,
    const Extent2D&         extent,
    VkFormat                format,
    VkImageAspectFlags      aspectFlags,
    VkSampleCountFlagBits   sampleCountBits,
    VkImageUsageFlags       usageFlags)
{
    if (format == VK_FORMAT_UNDEFINED)
        return;

    /* Release previous allocation */
    Release();

    /* Store reference to new device memory manager */
    memoryMngr_ = &deviceMemoryMngr;

    /* Create depth-stencil image */
    CreateVkImage(
        /*device:*/             deviceMemoryMngr.GetVkDevice(),
        /*imageType:*/          VK_IMAGE_TYPE_2D,
        /*format:*/             format,
        /*extent:*/             VkExtent3D{ extent.width, extent.height, 1 },
        /*numMipLevels:*/       1,
        /*numArrayLayers:*/     1,
        /*createFlags:*/        0,
        /*sampleCountBits:*/    sampleCountBits,
        /*usageFlags:*/         usageFlags
    );

    /* Allocate device memory region */
    AllocateMemoryRegion(deviceMemoryMngr);

    /* Create depth-stencil image view */
    VkImageSubresourceRange subresourceRange;
    {
        subresourceRange.aspectMask     = aspectFlags;
        subresourceRange.baseMipLevel   = 0;
        subresourceRange.levelCount     = 1;
        subresourceRange.baseArrayLayer = 0;
        subresourceRange.layerCount     = 1;
    }
    CreateVkImageView(deviceMemoryMngr.GetVkDevice(), VK_IMAGE_VIEW_TYPE_2D, format, subresourceRange, imageView_);

    /* Store parameters */
    format_ = format;
}

void VKRenderBuffer::Release()
{
    if (format_ != VK_FORMAT_UNDEFINED && memoryMngr_ != nullptr)
    {
        /* Release image and image view of depth-stencil buffer */
        imageView_.Release();
        ReleaseVkImage();

        /* Release device memory region of depth-stencil buffer */
        ReleaseMemoryRegion(*memoryMngr_);

        /* Reset depth-stencil format */
        format_ = VK_FORMAT_UNDEFINED;
    }
}


} // /namespace LLGL



// ================================================================================
