/*
 * VKDepthStencilBuffer.cpp
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#include "VKDepthStencilBuffer.h"
#include "../Memory/VKDeviceMemoryManager.h"


namespace LLGL
{


VKDepthStencilBuffer::VKDepthStencilBuffer(const VKPtr<VkDevice>& device) :
    VKDeviceImage { device                     },
    imageView_     { device, vkDestroyImageView }
{
}

// see https://www.khronos.org/registry/vulkan/specs/1.1-extensions/man/html/VkFormat.html
static VkImageAspectFlags GetVkImageAspectByFormat(VkFormat format)
{
    switch (format)
    {
        case VK_FORMAT_D16_UNORM:           return VK_IMAGE_ASPECT_DEPTH_BIT;
        case VK_FORMAT_X8_D24_UNORM_PACK32: return VK_IMAGE_ASPECT_DEPTH_BIT;
        case VK_FORMAT_D32_SFLOAT:          return VK_IMAGE_ASPECT_DEPTH_BIT;
        case VK_FORMAT_S8_UINT:             return VK_IMAGE_ASPECT_STENCIL_BIT;
        case VK_FORMAT_D16_UNORM_S8_UINT:   return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        case VK_FORMAT_D24_UNORM_S8_UINT:   return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        case VK_FORMAT_D32_SFLOAT_S8_UINT:  return VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
        default:                            return 0;
    }
}

void VKDepthStencilBuffer::CreateDepthStencil(
    VKDeviceMemoryManager& deviceMemoryMngr, const Extent2D& extent, VkFormat format, VkSampleCountFlagBits samplesFlags)
{
    /* Determine image aspect */
    auto imageAspect = GetVkImageAspectByFormat(format);
    if (!imageAspect)
        throw std::invalid_argument("invalid format for Vulkan depth-stencil buffer");

    auto device = deviceMemoryMngr.GetVkDevice();

    /* Create depth-stencil image */
    CreateVkImage(
        device,
        VK_IMAGE_TYPE_2D,
        format,
        { extent.width, extent.height, 1 },
        1,                                          // MIP levels
        1,                                          // array layers
        0,                                          // create flags
        samplesFlags,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
    );

    /* Allocate device memory region */
    AllocateMemoryRegion(deviceMemoryMngr);

    /* Create depth-stencil image view */
    CreateVkImageView(
        device,
        VK_IMAGE_VIEW_TYPE_2D,
        format,
        imageAspect,
        0, 1,                                       // MIP levels
        0, 1,                                       // array layers
        imageView_.ReleaseAndGetAddressOf()
    );

    /* Store parameters */
    format_ = format;
}

void VKDepthStencilBuffer::ReleaseDepthStencil(VKDeviceMemoryManager& deviceMemoryMngr)
{
    if (format_ != VK_FORMAT_UNDEFINED)
    {
        /* Release image and image view of depth-stencil buffer */
        imageView_.Release();
        ReleaseVkImage();

        /* Release device memory region of depth-stencil buffer */
        ReleaseMemoryRegion(deviceMemoryMngr);

        /* Reset depth-stencil format */
        format_ = VK_FORMAT_UNDEFINED;
    }
}


} // /namespace LLGL



// ================================================================================
