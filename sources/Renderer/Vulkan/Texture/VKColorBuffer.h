/*
 * VKColorBuffer.h
 *
 * Copyright (c) 2015 Lukas Hermanns. All rights reserved.
 * Licensed under the terms of the BSD 3-Clause license (see LICENSE.txt).
 */

#ifndef LLGL_VK_COLOR_BUFFER_H
#define LLGL_VK_COLOR_BUFFER_H


#include "VKRenderBuffer.h"


namespace LLGL
{


class VKColorBuffer final : private VKRenderBuffer
{

    public:

        VKColorBuffer() = default;
        VKColorBuffer(VkDevice device);

        // Explicit default move constructors required for GCC (to be used in VKSwapChain c'tor)
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
