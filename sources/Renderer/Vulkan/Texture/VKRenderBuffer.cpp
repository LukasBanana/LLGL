/*
 * VKRenderBuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKRenderBuffer.h"
#include "../Memory/VKDeviceMemoryManager.h"


namespace LLGL
{


VKRenderBuffer::VKRenderBuffer(const VKPtr<VkDevice>& device) :
    VKDeviceImage { device                     },
    imageView_    { device, vkDestroyImageView }
{
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
    VkSampleCountFlagBits   samplesCountBits,
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
        deviceMemoryMngr.GetVkDevice(),
        VK_IMAGE_TYPE_2D,
        format,
        VkExtent3D{ extent.width, extent.height, 1 },
        1,                                              // MIP levels
        1,                                              // array layers
        0,                                              // create flags
        samplesCountBits,
        usageFlags
    );

    /* Allocate device memory region */
    AllocateMemoryRegion(deviceMemoryMngr);

    /* Create depth-stencil image view */
    CreateVkImageView(
        deviceMemoryMngr.GetVkDevice(),
        VK_IMAGE_VIEW_TYPE_2D,
        format,
        aspectFlags,
        0, 1,                                           // MIP levels
        0, 1,                                           // array layers
        imageView_.ReleaseAndGetAddressOf()
    );

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
