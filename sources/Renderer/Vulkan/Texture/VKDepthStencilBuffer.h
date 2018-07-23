/*
 * VKDepthStencilBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2018 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_VK_DEPTH_STENCIL_BUFFER_H
#define LLGL_VK_DEPTH_STENCIL_BUFFER_H


#include "VKDeviceImage.h"


namespace LLGL
{


class VKDepthStencilBuffer final : private VKDeviceImage
{

    public:

        VKDepthStencilBuffer(const VKPtr<VkDevice>& device);

        void CreateDepthStencil(VKDeviceMemoryManager& deviceMemoryMngr, const Extent2D& extent, VkFormat format, VkSampleCountFlagBits samplesFlags);
        void ReleaseDepthStencil(VKDeviceMemoryManager& deviceMemoryMngr);

        // Returns the Vulkan image object.
        inline VkImage GetVkImage() const
        {
            return VKDeviceImage::GetVkImage();
        }

        // Returns the region of the hardware device memory.
        inline VKDeviceMemoryRegion* GetMemoryRegion() const
        {
            return VKDeviceImage::GetMemoryRegion();
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

    private:

        VKPtr<VkImageView>  imageView_;
        VkFormat            format_     = VK_FORMAT_UNDEFINED;

};


} // /namespace LLGL


#endif



// ================================================================================
