/*
 * VKDepthStencilBuffer.cpp
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#include "VKDepthStencilBuffer.h"
#include "../Memory/VKDeviceMemoryManager.h"


namespace LLGL
{


VKDepthStencilBuffer::VKDepthStencilBuffer(VkDevice device) :
    VKRenderBuffer { device }
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

void VKDepthStencilBuffer::Create(
    VKDeviceMemoryManager&  deviceMemoryMngr,
    const Extent2D&         extent,
    VkFormat                format,
    VkSampleCountFlagBits   sampleCountBits)
{
    /* Determine image aspect */
    const VkImageAspectFlags aspectFlags = GetVkImageAspectByFormat(format);
    LLGL_ASSERT(aspectFlags != 0, "invalid format (%d) for Vulkan depth-stencil buffer", static_cast<int>(format));

    /* Create depth-stencil image */
    VKRenderBuffer::Create(
        deviceMemoryMngr,
        extent,
        format,
        aspectFlags,
        sampleCountBits,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT
    );
}

void VKDepthStencilBuffer::Release()
{
    VKRenderBuffer::Release();
}


} // /namespace LLGL



// ================================================================================
