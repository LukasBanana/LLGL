/*
 * VKColorBuffer.h
 * 
 * This file is part of the "LLGL" project (Copyright (c) 2015-2019 by Lukas Hermanns)
 * See "LICENSE.txt" for license information.
 */

#ifndef LLGL_VK_COLOR_BUFFER_H
#define LLGL_VK_COLOR_BUFFER_H


#include "VKRenderBuffer.h"


namespace LLGL
{


class VKColorBuffer final : private VKRenderBuffer
{

    public:

        VKColorBuffer(const VKPtr<VkDevice>& device);

        // Explicit default move constructors required for GCC (to be used in VKRenderContext c'tor)
        VKColorBuffer(VKColorBuffer&&) = default;
        VKColorBuffer& operator = (VKColorBuffer&&) = default;

        void Create(
            VKDeviceMemoryManager&  deviceMemoryMngr,
            const Extent2D&         extent,
            VkFormat                format,
            VkSampleCountFlagBits   sampleCountBits
        );

        void Release();

        // Returns the Vulkan image object.
        inline VkImage GetVkImage() const
        {
            return VKRenderBuffer::GetVkImage();
        }

        // Returns the Vulkan image object.
        inline VkImageView GetVkImageView() const
        {
            return VKRenderBuffer::GetVkImageView();
        }

        // Returns the VkFormat with whereby the VkImage object was created.
        inline VkFormat GetVkFormat() const
        {
            return VKRenderBuffer::GetVkFormat();
        }

};


} // /namespace LLGL


#endif



// ================================================================================
