/*
 * VKRenderBuffer.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_VK_RENDER_BUFFER_H
#define LLGL_VK_RENDER_BUFFER_H


#include "VKDeviceImage.h"


namespace LLGL
{


// Base class for VKDepthStencilBuffer and VKColorBuffer used as framebuffer attachments.
class VKRenderBuffer : private VKDeviceImage
{

    public:

        VKRenderBuffer(VkDevice device);
        ~VKRenderBuffer();

        // Explicit default move constructors required for GCC (to be used in VKSwapChain c'tor)
        VKRenderBuffer(VKRenderBuffer&&) = default;
        VKRenderBuffer& operator = (VKRenderBuffer&&) = default;

        void Create(
            VKDeviceMemoryManager&  deviceMemoryMngr,
            const Extent2D&         extent,
            VkFormat                format,
            VkImageAspectFlags      aspectFlags,
            VkSampleCountFlagBits   samplesCountBits,
            VkImageUsageFlags       usageFlags
        );

        void Release();

        // Returns the Vulkan image object.
        inline VkImage GetVkImage() const
        {
            return VKDeviceImage::GetVkImage();
        }

        // Returns the Vulkan image object.
        inline VkImageView GetVkImageView() const
        {
            return imageView_;
        }

        // Returns the VkFormat with whereby the VkImage object was created.
        inline VkFormat GetVkFormat() const
        {
            return format_;
        }

        // Returns the region of the hardware device memory.
        inline VKDeviceMemoryRegion* GetMemoryRegion() const
        {
            return VKDeviceImage::GetMemoryRegion();
        }

    private:

        VKPtr<VkImageView>      imageView_;
        VkFormat                format_     = VK_FORMAT_UNDEFINED;
        VKDeviceMemoryManager*  memoryMngr_ = nullptr;

};


} // /namespace LLGL


#endif



// ================================================================================
